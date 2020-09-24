//*************************************************************************
//    Toppers_ASP on STM32F401_Nucleo-64 :  User_Task_Template
//														    		 Sumida
//*************************************************************************
//***** Include Header Files **********************************************
#include "kernel_cfg.h"             // Toppers Kernel Config Header
#include "hal.h"                    // Toppers Header
#include "hal_extention.h"          // Toppers HAL_Extention Header
#include "mailbox_packet_struct.h"
//#include "my_servo.h"
//#include "my_PWM_control.h"
#include <stdio.h>
//***** Define ************************************************************
//#define DEBUG//課題のやつのスイッチ
//#define MY_DEBUG//opの値、GETで得たパケットアドレスを表示する。
#define GET 0
#define REL 1
#define CLR 2
#define ERR -1
#define OK 0
#define PACKET_LIST_NUM 20//パケットリスト配列の要素数。
//***** Task **************************************************************

void task1(intptr_t exinf){
	sio_open(TASK_PORTID);
	char s_toID[7]={0,0,0,0,0,0,0};
	char s_myTaskID[2]={0,0};
	char s_packetAddr[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	MBX_PKT* packet_out;
	MBX_PKT* packet_in;
	T_RMBX mbx_status;
	unsigned int myTaskID=(unsigned int)exinf;

	while(1){
		//メール文書（パケット）を獲得
		if(get_mpf(MPF,(void**)&packet_out)!=E_OK){
			sio_write_string(TASK_PORTID,"ERR:get_mpf\n\r");

		}
		//獲得したパケットのアドレスをパケット管理配列に追加---
		if(mpf_manager(GET,packet_out)==ERR){
			sio_write_string(TASK_PORTID,"mpfGET ERR\n\r");
			return;
		}

		while(1){
			//初期メッセージ表示
			sio_write_string(TASK_PORTID,"\n\r");
			sio_write_string(TASK_PORTID,"TSK");
			sprintf(s_myTaskID,"%d",myTaskID);
			sio_write_string(TASK_PORTID,s_myTaskID);
			sio_write_string(TASK_PORTID,": Make Mail_Message \n\r");

			//自分のTASKIDを格納
			packet_out->fromID=myTaskID;

			//宛先入力メッセージ
			sio_write_string(TASK_PORTID,"TSK");
			sio_write_string(TASK_PORTID,s_myTaskID);
			sio_write_string(TASK_PORTID,": To whom? (TSK2/TSK3/end) > ");

			//宛先のTASKIDを入力・格納
			sio_read_string(TASK_PORTID,s_toID);
			if(strcmp(s_toID,"TSK2")==0){
				packet_out->toID=2;
				break;
			}else if(strcmp(s_toID,"TSK3")==0){
				packet_out->toID=3;
				break;
			}else if(strcmp(s_toID,"end")==0){
				sio_write_string(TASK_PORTID,"Program End\n\r");
				break;
			}
			else{
				sio_write_string(TASK_PORTID,"No such guy, Retry\n\r");
				continue;
			}
		}
		if(strcmp(s_toID,"end")==0){
			break;
		}

		//パケットアドレス表示--debug--
#ifdef DEBUG
		sio_write_string(TASK_PORTID,"GET PACKET ADDR =");
		sprintf(s_packetAddr,"%p",packet_out);
		sio_write_string(TASK_PORTID,s_packetAddr);
		sio_write_string(TASK_PORTID,"\n\r");
#endif

		//メール本文入力メッセージ
		sio_write_string(TASK_PORTID,"TSK");
		sio_write_string(TASK_PORTID,s_myTaskID);
		sio_write_string(TASK_PORTID,": message > ");

		//メール本文入力
		sio_read_string(TASK_PORTID,packet_out->data);

		//メール送信
		if(packet_out->toID==2){
			snd_mbx(MBX2,(T_MSG*)packet_out);                  // MBX2へ送信
		}else if(packet_out->toID==3){
			snd_mbx(MBX3,(T_MSG*)packet_out);                  // MBX3へ送信
		}

		//自分宛のメールがくるまで待ち
		while(1){
			ref_mbx(MBX1,&mbx_status);
			if(mbx_status.pk_msg!=NULL){
				packet_in=(MBX_PKT*)(mbx_status.pk_msg);
				if(packet_in->toID==myTaskID){
					//メール受信
					rcv_mbx(MBX1,(T_MSG**)&packet_in);            // MBX1受信
					break;
				}
			}
			dly_tsk(1);
		}

		//受信メール文書data表示
		sio_write_string(TASK_PORTID,"TSK");
		sio_write_string(TASK_PORTID,s_myTaskID);
		sio_write_string(TASK_PORTID,": Recv: ");
		sio_write_string(TASK_PORTID,packet_in->data);
		sio_write_string(TASK_PORTID,"\n\r");

#ifndef DEBUG
		//解放するパケットのアドレスをパケット管理配列から削除---
		if(mpf_manager(REL,packet_in)==ERR){
			return;
		}
		//受信メール文書（パケット）解放
		if(rel_mpf(MPF,packet_in)!=E_OK){
			sio_write_string(TASK_PORTID,"ERR:rel_mpf\n\r");
		}
#endif
	}

#ifdef DEBUG
	//パケット管理配列から、解放されていない全受信メール文書（パケット）解放---
	mpf_manager(CLR);
#endif
	ter_tsk(TASK2);
	ter_tsk(TASK3);
}

void task2(intptr_t exinf){
	MBX_PKT* packet_in;
	MBX_PKT* packet_out;
	char s_ID[2]={0,0};
	char s_fromID[2]={0,0};
	int outFromID=0;
	T_RMBX mbx_status;
	unsigned int myTaskID = (unsigned int)exinf;

	while(1){

		//自分宛のメールがくるまで待ち
		while(1){
			ref_mbx(MBX2,&mbx_status);
			if(mbx_status.pk_msg!=NULL){
				packet_in=(MBX_PKT*)(mbx_status.pk_msg);
				if(packet_in->toID==myTaskID){
					//メール受信
					rcv_mbx(MBX2,(T_MSG**)&packet_in);            // MBX1受信
					break;
				}
			}
			dly_tsk(1);
		}

		//メール文書data表示
		sio_write_string(TASK_PORTID,"TSK");
		sprintf(s_ID,"%d",exinf);
		sio_write_string(TASK_PORTID,s_ID);
		sio_write_string(TASK_PORTID,": Recv: ");
		sio_write_string(TASK_PORTID,packet_in->data);
		sio_write_string(TASK_PORTID,"\n\r");
		sprintf(s_fromID,"%d",packet_in->fromID);


		outFromID=packet_in->fromID;

#ifndef DEBUG
		//解放するパケットのアドレスをパケット管理配列から削除---
		if(mpf_manager(REL,packet_in)==ERR){
			return;
		}
		//受信メール文書（パケット）解放
		if(rel_mpf(MPF,packet_in)!=E_OK){
			sio_write_string(TASK_PORTID,"ERR:rel_mpf\n\r");
			return;
		}
#endif
		//送信メール文書（パケット）獲得
		if(get_mpf(MPF,(void**)&packet_out)!=E_OK){
			sio_write_string(TASK_PORTID,"ERR:get_mpf\n\r");
			return;
		}

		//獲得したパケットのアドレスをパケット管理配列に追加---
		if(mpf_manager(GET,packet_out)==ERR){
			return;
		}

		//メール返信メッセージ
		sio_write_string(TASK_PORTID,"TSK");
		sio_write_string(TASK_PORTID,s_ID);
		sio_write_string(TASK_PORTID,": Reply to TSK");
		sio_write_string(TASK_PORTID,s_fromID);
		sio_write_string(TASK_PORTID," >");

		//返信文書入力
		sio_read_string(TASK_PORTID,packet_out->data);

		//宛先のTASKIDを格納
		packet_out->toID=outFromID;

		//自分のTASKIDを格納
		packet_out->fromID=(unsigned int)exinf;

		//メール送信
		if(packet_out->toID==1){
			snd_mbx(MBX1,(T_MSG*)packet_out);                  // MBX1へ送信
		}else if(packet_out->toID==3){
			snd_mbx(MBX3,(T_MSG*)packet_out);                  // MBX3へ送信
		}else{
			sio_write_string(TASK_PORTID,"toID error\n\r");
		}

	}

}

void task3(intptr_t exinf){
	MBX_PKT* packet_in;
	MBX_PKT* packet_out;
	char s_ID[2]={0,0};
	char s_fromID[2]={0,0};
	int outFromID=0;
	T_RMBX mbx_status;
	unsigned int myTaskID = (unsigned int)exinf;

	while(1){
		//自分宛のメールがくるまで待ち
		while(1){
			ref_mbx(MBX3,&mbx_status);
			if(mbx_status.pk_msg!=NULL){
				packet_in=(MBX_PKT*)(mbx_status.pk_msg);
				if(packet_in->toID==myTaskID){
					//メール受信
					rcv_mbx(MBX3,(T_MSG**)&packet_in);            // MBX1受信
					break;
				}
			}
			dly_tsk(1);
		}

		//メール文書data表示
		sio_write_string(TASK_PORTID,"TSK");
		sprintf(s_ID,"%d",exinf);
		sio_write_string(TASK_PORTID,s_ID);
		sio_write_string(TASK_PORTID,": Recv: ");
		sio_write_string(TASK_PORTID,packet_in->data);
		sio_write_string(TASK_PORTID,"\n\r");
		sprintf(s_fromID,"%d",packet_in->fromID);

		outFromID=packet_in->fromID;

#ifndef DEBUG
		//解放したパケットのアドレスをパケット管理配列から削除---
		if(mpf_manager(REL,packet_in)==ERR){
			return;
		}
		//受信メール文書（パケット）解放
		if(rel_mpf(MPF,packet_in)!=E_OK){
			sio_write_string(TASK_PORTID,"ERR:rel_mpf\n\r");
			return;
		}
		
#endif
		//送信メール文書（パケット）獲得
		if(get_mpf(MPF,(void**)&packet_out)!=E_OK){
			return;
		}

		//獲得したパケットのアドレスをパケット管理配列に追加---
		if(mpf_manager(GET,packet_out)==ERR){
			return;
		}

		//メール返信メッセージ
		sio_write_string(TASK_PORTID,"TSK");
		sio_write_string(TASK_PORTID,s_ID);
		sio_write_string(TASK_PORTID,": Reply to TSK");
		sio_write_string(TASK_PORTID,s_fromID);
		sio_write_string(TASK_PORTID," >");

		//返信文書入力
		sio_read_string(TASK_PORTID,packet_out->data);

		//宛先のTASKIDを格納
		packet_out->toID=outFromID;

		//自分のTASKIDを格納
		packet_out->fromID=(unsigned int)exinf;

		//メール送信
		if(packet_out->toID==1){
			snd_mbx(MBX1,(T_MSG*)packet_out);                  // MBX1へ送信
		}else if(packet_out->toID==3){
			snd_mbx(MBX2,(T_MSG*)packet_out);                  // MBX2へ送信
		}else{
			sio_write_string(TASK_PORTID,"toID error\n\r");
		}

	}


}

int mpf_manager(int op,...){
	static MBX_PKT* packet_list[PACKET_LIST_NUM];
	char s_packetAddr[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static int packet_index=0;
	int i=0;
	int j=0;
	MBX_PKT* packet;
	va_list arg_list;
	va_start(arg_list, op);
	char s_mode[5]={0,0,0,0,0};

#ifdef MY_DEBUG
	sprintf(s_mode,"\n\rMY_DEBUG: op=%d\n\r",op);
	sio_write_string(TASK_PORTID,s_mode);
#endif

	switch (op)
	{
	case GET:
		//引数のパケットがリストから見つかったらエラー。見つからなかったらリストに追加。
		
		if(packet_index >= (sizeof(packet_list)/sizeof(MBX_PKT*))){
			sio_write_string(TASK_PORTID,"ERR:mpf_manager:GET:packet list is full.\n\r");
			return ERR;
		}

		packet=va_arg(arg_list,MBX_PKT*);
		va_end(arg_list);

#ifdef MY_DEBUG
		sio_write_string(TASK_PORTID,"MY_DEBUG: GET PACKET ADDR :");
		sprintf(s_packetAddr,"%p\n\r",packet);
		sio_write_string(TASK_PORTID,s_packetAddr);
		memset(s_packetAddr,0,sizeof(s_packetAddr));
#endif
	
		for(i=0; packet_list[i]!=packet && i<=packet_index; i++){}
		if(i<=packet_index){
			sio_write_string(TASK_PORTID,"ERR:mpf_manager:GET:This packet is already in packet list.\n\r");
			return ERR;
		}
		packet_list[packet_index]=packet;
		packet_index++;
		break;

	case REL:
		packet=va_arg(arg_list,MBX_PKT*);
		va_end(arg_list);

#ifdef MY_DEBUG	
		sio_write_string(TASK_PORTID,"MY_DEBUG: REL PACKET ADDR :");
		sprintf(s_packetAddr,"%p\n\r",packet);
		sio_write_string(TASK_PORTID,s_packetAddr);
		memset(s_packetAddr,0,sizeof(s_packetAddr));
#endif
	//引数のパケットがリストから見つかったらリストから削除。見つからなかったらエラー返す。
		for(i=0; packet_list[i]!=packet && i<=packet_index; i++){}
		if(i > packet_index){
			sio_write_string(TASK_PORTID,"ERR:mpf_manager:REL:This packet is not.\n\r");
			return ERR;
		}
		packet_list[i]=NULL;	
		//リストを左詰め
		for(;i<packet_index;i++){
			packet_list[i]=packet_list[i+1];
		}
		packet_index--;

		break;

	case CLR:
		for(j=0; j<packet_index; j++){
			sio_write_string(TASK_PORTID,"RLS PACKET ADDR :");
			sprintf(s_packetAddr,"%p\n\r",packet_list[j]);
			sio_write_string(TASK_PORTID,s_packetAddr);
			//受信メール文書（パケット）解放
			if(rel_mpf(MPF,packet_list[j])!=E_OK){
				sio_write_string(TASK_PORTID,"ERR:rel_mpf\n\r");
				return ERR;
			}	
			memset(s_packetAddr,0,sizeof(s_packetAddr));
		}

		memset(packet_list,0,sizeof(packet_list));
		break;

	default:
		sio_write_string(TASK_PORTID,"switch default\n\r");
		break;
	}
	va_end(arg_list);
	return OK;

}
