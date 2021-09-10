#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <sqlite3.h>
#include <time.h>
#define TRUE   1
#define FALSE  0

int result;
int sd;
char id[1000];
struct Login_User
{
	char username[256];
}login_user[30];

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   result = argc;
   // for(i=0; i<argc; i++){
   //    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   // }
   // printf("\n");
   return 0;
}
static int list(void *NotUsed, int argc, char **argv, char **azColName)
{
	char message[1024] = "";
	for(int i=0; i<argc; i++)
	{
		strcat(message, "\t");
		strcat(message, argv[i]);
		strcat(message, "\t\t");
	}
	strcat(message, "\n");
	send(sd, message, strlen(message), 0);
	return 0;
}
static int get_id(void *NotUsed, int argc, char **argv, char **azColName)
{
	result = argc;
	memset(id,'\0',sizeof(id));
	for(int i=0;i<strlen(argv[0]);i++)
	{
		id[i] = argv[0][i];
	}
	return 0;
}
static int list_post(void *NotUsed, int argc, char **argv, char **azColName)
{
	char message[1024] = "";
	
	strcat(message, "\t");
	strcat(message, "Author");
	strcat(message, "\t\t:");
	strcat(message, argv[0]);
	strcat(message, "\n");

	strcat(message, "\t");
	strcat(message, "Title");
	strcat(message, "\t\t:");
	strcat(message, argv[1]);
	strcat(message, "\n");

	strcat(message, "\t");
	strcat(message, "Date");
	strcat(message, "\t\t:");
	strcat(message, argv[2]);
	strcat(message, "\n");
	
	strcat(message, "\t--\n");
	send(sd, message, strlen(message), 0);
	send(sd, "\t", strlen("\t"), 0);
	for(int i=0;i<strlen(argv[3]);i++)
	{
		if(argv[3][i] == '<' && argv[3][i+1] == 'b' && argv[3][i+2] == 'r' && argv[3][i+3] == '>')
		{
			send(sd, "\n", strlen("\n"), 0);
			send(sd, "\t", strlen("\t"), 0);
			i = i + 3;
		}
		else
		{
			send(sd, &argv[3][i], 1, 0);
		}
	}
	send(sd, "\n\t--\n", strlen("\n\t--\n"), 0);
	return 0;
}
static int list_comment(void *NotUsed, int argc, char **argv, char **azColName)
{
	if(argc == 0)
	{
		return 0;
	}
	send(sd, "\t", strlen("\t"), 0);
	send(sd, argv[2], strlen(argv[2]), 0);
	send(sd, ": ", strlen(": "), 0);
	send(sd, argv[1], strlen(argv[1]), 0);
	send(sd, "\n", strlen("\n"), 0);
	return 0;
}
int main(int argc, char **argv)
{
	sqlite3 *db;
	int rc;
	rc = sqlite3_open("hw2", &db);
	// if( rc )
 //    {
 //    	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	// 	exit(0);
	// }
	// else
	// {
	// 	fprintf(stdout, "Opened database successfully\n");
	// }
	char *sql;
	char *zErrMsg = 0;
	sql = "CREATE TABLE USER(USERNAME TEXT, EMAIL TEXT, PASSWORD TEXT);";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	sql = "CREATE TABLE BOARDLIST(ID INTEGER PRIMARY KEY, NAME TEXT, MODERATOR TEXT);";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    sql = "CREATE TABLE POSTLIST(ID INTEGER PRIMARY KEY, TITLE TEXT, AUTHOR TEXT, DATE TEXT, BOARDNAME TEXT);";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    sql = "CREATE TABLE COMMENTLIST(POSTID TEXT, COMMENT TEXT, USERNAME TEXT);";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   		
   	int port_number = atoi(argv[1]);
	int opt = TRUE;
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30 ,activity, valread;
    int max_sd;
    struct sockaddr_in info;


    char buffer[1025];
	fd_set readfds;
	char *welcome_message = "********************************\n" \
							"** Welcome to the BBS server. **\n" \
							"********************************\n" \
							"% ";
	
	for (int i=0;i<max_clients;i++)
	{
		client_socket[i] = 0;
	}
	if((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	bzero(&info,sizeof(info));
	info.sin_family = AF_INET;
	info.sin_addr.s_addr = INADDR_ANY;
	info.sin_port = htons(port_number);

	if(bind(master_socket, (struct sockaddr *)&info, sizeof(info)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if(listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	addrlen = sizeof(info);

	for(int i=0;i<30;i++)
	{
		memset(login_user[i].username, '\0', 256);
	}
	while(TRUE)
	{
		FD_ZERO(&readfds);
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
		for (int i=0;i<max_clients;i++)
		{
			//socket descriptor  
			sd = client_socket[i];

			//if valid socket descriptor then add to read list  
			if(sd > 0)
				FD_SET( sd , &readfds);

			//highest file descriptor number, need it for the select function  
			if(sd > max_sd)
				max_sd = sd;
		}
		//wait for an activity on one of the sockets , timeout is NULL ,  
		//so wait indefinitely  
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

		if((activity < 0) && (errno!=EINTR))
		{
			printf("select error");
		}

		//If something happened on the master socket ,  
		//then its an incoming connection  
		if(FD_ISSET(master_socket, &readfds))
		{
			if ((new_socket = accept(master_socket, (struct sockaddr *)&info, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
			//inform user of socket number - used in send and receive commands  
			printf("New connection.\n");

			//send new connection greeting message  
			if(send(new_socket, welcome_message, strlen(welcome_message), 0) != strlen(welcome_message))
			{
				perror("send");
			}

			

			for (int i = 0; i < max_clients; i++)
			{
				if(client_socket[i] == 0)
				{
					client_socket[i] = new_socket;
					
					break;
				}
			}
		}
		int socket_index;
		for(socket_index=0;socket_index<max_clients;socket_index++)
		{
			result = 0;
			sd = client_socket[socket_index];
			if(FD_ISSET(sd , &readfds))
			{
				if((valread = read(sd, buffer, 1024)) == 0)
				{
					printf("%d disconnect\n", socket_index);
					close(sd);
					client_socket[socket_index] = 0;
					memset(login_user[socket_index].username, '\0', 256);
				}
				else
				{
					buffer[valread] = '\0';
					if(buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'i' && buffer[3] == 't')
					{
						close(sd);
						client_socket[socket_index] = 0;
						memset(login_user[socket_index].username, '\0', 256);
					}
					else if(buffer[0] == 'r' && buffer[1] == 'e' && buffer[2] == 'g' && buffer[3] == 'i' && buffer[4] == 's' && buffer[5] == 't' && buffer[6] == 'e' && buffer[7] == 'r')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 3)
						{
							char *message = "Usage: register <username> <email> <password>\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							char username[256], email[256], password[256];
							int i, j, k, index=0;
							for(i=9;i<strlen(buffer);i++)
							{
								if(buffer[i] == ' ')
								{
									break;
								}
								username[index] = buffer[i];
								index++;
							}
							username[index] = '\0';
							index = 0;
							for(j=i+1;j<strlen(buffer);j++)
							{
								if(buffer[j] == ' ')
								{
									break;
								}
								email[index] = buffer[j];
								index++;
							}
							email[index] = '\0';
							index = 0;
							for(k=j+1;k<strlen(buffer);k++)
							{
								if(buffer[k] == '\r' || buffer[k] == '\n')
								{
									break;
								}
								password[index] = buffer[k];
								index++;
							}
							password[index] = '\0';
							// printf("%s %s %s\n", username, password, email);
							char sql[256] = "";
							strcat(sql, "SELECT * ");
							strcat(sql, " from USER WHERE USERNAME='");
							strcat(sql, username);
							strcat(sql, "';");
							rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
							// if(rc != SQLITE_OK)
							// {
							// 	fprintf(stderr, "SQL error: %s\n", zErrMsg);
							// 	sqlite3_free(zErrMsg);
							// }
							// else
							// {
							// 	fprintf(stdout, "Operation done successfully\n");
							// }
							// printf("%d", result);
							if(result != 0)
							{
								char *message = "Username is already used.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char sql[256] = "";
								strcat(sql, "INSERT INTO USER (USERNAME, EMAIL, PASSWORD) VALUES ('");
								strcat(sql, username);
								strcat(sql, "', '");
								strcat(sql, email);
								strcat(sql, "', '");
								strcat(sql, password);
								strcat(sql, "'); ");
								rc = sqlite3_exec(db, sql, callback, "0", &zErrMsg);
								// if( rc != SQLITE_OK )
								// {
								// 	fprintf(stderr, "SQL error5: %s\n", zErrMsg);
								// 	sqlite3_free(zErrMsg);
								// }
								// else
								// {
								// 	fprintf(stdout, "Operation done successfully\n");
								// }
								char *message = "Register successfully.\n";
								send(sd, message, strlen(message), 0);
							}
						}
					}
					else if(buffer[0] == 'l' && buffer[1] == 'o' && buffer[2] == 'g' && buffer[3] == 'i' && buffer[4] == 'n')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 2)
						{
							char *message = "Usage: login <username> <password>\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							if(strlen(login_user[socket_index].username) != 0)
							{
								char *message = "Please logout first.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char username[256], password[256];
								int i, j, index=0;
								for(i=6;i<strlen(buffer);i++)
								{
									if(buffer[i] == ' ')
									{
										break;
									}
									username[index] = buffer[i];
									index++;
								}
								username[index] = '\0';
								index = 0;
								for(j=i+1;j<strlen(buffer);j++)
								{
									if(buffer[j] == '\r' || buffer[j] == '\n')
									{
										break;
									}
									password[index] = buffer[j];
									index++;
								}
								password[index] = '\0';
								char sql[256] = "";
								strcat(sql, "SELECT * ");
								strcat(sql, "from USER WHERE USERNAME='");
								strcat(sql, username);
								strcat(sql, "' AND PASSWORD='");
								strcat(sql, password);
								strcat(sql, "';");
								rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
								
								if(result == 0)
								{
									char *message = "Login failed.\n";
									send(sd, message, strlen(message), 0);
								}
								else
								{
									strcpy(login_user[socket_index].username, username);
									char message[256] = "";
									strcat(message, "Welcome, ");
									strcat(message, username);
									strcat(message, ".\n");
									send(sd, message, strlen(message), 0);
								}
							}
						}
					}
					else if(buffer[0] == 'l' && buffer[1] == 'o' && buffer[2] == 'g' && buffer[3] == 'o' && buffer[4] == 'u' && buffer[5] == 't')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 0)
						{
							char *message = "Usage: logout\n";
							send(sd, message, strlen(message), 0);
						}
						if(strlen(login_user[socket_index].username) == 0)
						{
							char *message = "Please login first.\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							char *message = "Bye, ";
							send(sd, message, strlen(message), 0);
							message = login_user[socket_index].username;
							send(sd, message, strlen(message), 0);
							message = ".\n";
							send(sd, message, strlen(message), 0);
							memset(login_user[socket_index].username, '\0', 256);
						}
					}
					else if(buffer[0] == 'w' && buffer[1] == 'h' && buffer[2] == 'o' && buffer[3] == 'a' && buffer[4] == 'm' && buffer[5] == 'i')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 0)
						{
							char *message = "Usage: whoami\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							if(strlen(login_user[socket_index].username) == 0)
							{
								char *message = "Please login first.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char *message = login_user[socket_index].username;
								send(sd, message, strlen(message), 0);
								message = "\n";
								send(sd, message, strlen(message), 0);
							}
						}
					}
					else if(buffer[0] == 'c' && buffer[1] == 'r' && buffer[2] == 'e' && buffer[3] == 'a' && buffer[4] == 't' && buffer[5] == 'e' && buffer[6] == '-' && buffer[7] == 'b' && buffer[8] == 'o' && buffer[9] == 'a' && buffer[10] == 'r' && buffer[11] == 'd')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 1)
						{
							char *message = "Usage: create-board <name>\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							if(strlen(login_user[socket_index].username) == 0)
							{
								char *message = "Please login first.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char board_name[256];
								int i, index=0;
								for(i=13;i<strlen(buffer);i++)
								{
									if(buffer[i] == '\r' || buffer[i] == '\n')
									{
										break;
									}
									board_name[index] = buffer[i];
									index++;
								}
								board_name[index] = '\0';
								char sql[256] = "";
								strcat(sql, "SELECT * ");
								strcat(sql, "from BOARDLIST WHERE NAME='");
								strcat(sql, board_name);
								strcat(sql, "';");
								rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
								if(result != 0)
								{
									char *message = "Board is already exist.\n";
									send(sd, message, strlen(message), 0);
								}
								else
								{
									char sql[256] = "";
									strcat(sql, "CREATE TABLE '");
									strcat(sql, board_name);
									strcat(sql, "' (ID TEXT, AUTHOR TEXT, TITLE TEXT, DATE TEXT, CONTENT TEXT);");
									rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
									char *message = "Create board successfully.\n";
									send(sd, message, strlen(message), 0);
									char sql2[256] = "";
									strcat(sql2, "INSERT INTO BOARDLIST (NAME, MODERATOR) VALUES ('");
									strcat(sql2, board_name);
									strcat(sql2, "', '");
									strcat(sql2, login_user[socket_index].username);
									strcat(sql2, "'); ");
									rc = sqlite3_exec(db, sql2, callback, "0", &zErrMsg);
								}
							}
						}
					}
					else if(buffer[0] == 'c' && buffer[1] == 'r' && buffer[2] == 'e' && buffer[3] == 'a' && buffer[4] == 't' && buffer[5] == 'e' && buffer[6] == '-' && buffer[7] == 'p' && buffer[8] == 'o' && buffer[9] == 's' && buffer[10] == 't')
					{
						if(strlen(login_user[socket_index].username) == 0)
						{
							char *message = "Please login first.\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							char board_name[256], title[1000], content[1000];
							int i, j, k, index=0;
							for(i=12;i<strlen(buffer);i++)
							{
								if(buffer[i] == '\r' || buffer[i] == '\n' || buffer[i] == ' ')
								{
									break;
								}
								board_name[index] = buffer[i];
								index++;
							}
							board_name[index] = '\0';
							index = 0;
							if (buffer[i+1] != '-' || buffer[i+2] != '-' || buffer[i+3] != 't' || buffer[i+4] != 'i' || buffer[i+5] != 't' || buffer[i+6] != 'l' || buffer[i+7] != 'e' || buffer[i+8] != ' ')
							{
								char *message = "Usage: create-post <board-name> --title <title> --content <content> \n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								for(j=i+9;j<strlen(buffer);j++)
								{
									if(buffer[j] == '\r' || buffer[j] == '\n' || (buffer[j] == ' ' && buffer[j+1] == '-' && buffer[j+2] == '-' && buffer[j+3] == 'c' && buffer[j+4] == 'o' && buffer[j+5] == 'n' && buffer[j+6] == 't' && buffer[j+7] == 'e' && buffer[j+8] == 'n' && buffer[j+9] == 't' && buffer[j+10] == ' '))
									{
										break;
									}
									title[index] = buffer[j];
									index++;
								}
								title[index] = '\0';
								index = 0;
								for(k=j+11;k<strlen(buffer);k++)
								{
									if(buffer[k] == '\r' || buffer[k] == '\n')
									{
										break;
									}
									content[index] = buffer[k];
									index++;
								}
								content[index] = '\0';
								char sql[256] = "";
								strcat(sql, "SELECT * ");
								strcat(sql, "FROM BOARDLIST WHERE NAME='");
								strcat(sql, board_name);
								strcat(sql, "';");
								rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
								if(result == 0)
								{
									char *message = "Board is not exist.\n";
									send(sd, message, strlen(message), 0);
								}
								else
								{
									char *message = "Create post successfully.\n";
									send(sd, message, strlen(message), 0);
									time_t t = time(NULL);
									struct tm tm = *localtime(&t);
									char sql2[256] = "";
									strcat(sql2, "INSERT INTO POSTLIST (TITLE, AUTHOR, DATE, BOARDNAME) VALUES ('");
									strcat(sql2, title);
									strcat(sql2, "', '");
									strcat(sql2, login_user[socket_index].username);
									strcat(sql2, "', '");
									char month_p[256];
									sprintf(month_p, "%02d", tm.tm_mon + 1);
									strcat(sql2, month_p);
									strcat(sql2, "/");
									char day_p[256];
									sprintf(day_p, "%02d", tm.tm_mday);
									strcat(sql2, day_p);
									strcat(sql2, "', '");
									strcat(sql2, board_name);
									strcat(sql2, "'); ");
									rc = sqlite3_exec(db, sql2, callback, "0", &zErrMsg);
									char sql3[256] = "SELECT MAX(ID) FROM POSTLIST";
									rc = sqlite3_exec(db, sql3, get_id, "0", &zErrMsg);
									char sql[256] = "";
									strcat(sql, "INSERT INTO '");
									strcat(sql, board_name);
									strcat(sql, "' (ID, AUTHOR, TITLE, DATE, CONTENT) VALUES ('");
									strcat(sql, id);
									strcat(sql, "', '");
									strcat(sql, login_user[socket_index].username);
									strcat(sql, "', '");
									strcat(sql, title);
									strcat(sql, "', '");
									char year[256];
									sprintf(year, "%d", tm.tm_year + 1900);
									strcat(sql, year);
									strcat(sql, "-");
									char month[256];
									sprintf(month, "%02d", tm.tm_mon + 1);
									strcat(sql, month);
									strcat(sql, "-");
									char day[256];
									sprintf(day, "%02d", tm.tm_mday);
									strcat(sql, day);
									strcat(sql, "', '");
									strcat(sql, content);
									strcat(sql, "'); ");
									rc = sqlite3_exec(db, sql, callback, "0", &zErrMsg);
								}
							}
						}
					}
					else if(buffer[0] == 'l' && buffer[1] == 'i' && buffer[2] == 's' && buffer[3] == 't' && buffer[4] == '-' && buffer[5] == 'b' && buffer[6] == 'o' && buffer[7] == 'a' && buffer[8] == 'r' && buffer[9] == 'd')
					{
						if(buffer[10] == '\r' || buffer[10] == '\n')
						{
							char *message = "\tIndex\t\t\tName\t\t\tModerator\n";
							send(sd, message, strlen(message), 0);
							char sql[256] = "";
							strcat(sql, "SELECT * ");
							strcat(sql, "FROM BOARDLIST");
							rc = sqlite3_exec(db, sql, list, "0", &zErrMsg);
						}
						else
						{
							char key[256] = "";
							int index = 0;
							for(int i=13;i<strlen(buffer);i++)
							{
								if(buffer[i] == '\r' || buffer[i] == '\n')
								{
									break;
								}
								key[index] = buffer[i];
								index++;
							}
							key[index] = '\0';
							char *message = "\tIndex\t\t\tName\t\t\tModerator\n";
							send(sd, message, strlen(message), 0);
							char sql[256] = "";
							strcat(sql, "PRAGMA case_sensitive_like=ON;SELECT * ");
							strcat(sql, "FROM BOARDLIST WHERE NAME LIKE '%");
							strcat(sql, key);
							strcat(sql, "%';");
							rc = sqlite3_exec(db, sql, list, "0", &zErrMsg);
						}
					}
					else if(buffer[0] == 'l' && buffer[1] == 'i' && buffer[2] == 's' && buffer[3] == 't' && buffer[4] == '-' && buffer[5] == 'p' && buffer[6] == 'o' && buffer[7] == 's' && buffer[8] == 't')
					{
						char board_name[256] = "";
						int i, index = 0;
						for(i=10;i<strlen(buffer);i++)
						{
							if(buffer[i] == '\r' || buffer[i] == '\n' || buffer[i] == ' ')
							{
								break;
							}
							board_name[index] = buffer[i];
							index++;
						}
						board_name[index] = '\0';
						if(buffer[i] == '\r' || buffer[i] == '\n')
						{
							char sql[256] = "";
							strcat(sql, "SELECT * ");
							strcat(sql, "from BOARDLIST WHERE NAME='");
							strcat(sql, board_name);
							strcat(sql, "';");
							rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
							if(result == 0)
							{
								char *message = "Board is not exist.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char *message = "\tID\t\t\tTitle\t\t\tAuthor\t\t\tDate\n";
								send(sd, message, strlen(message), 0);
								char sql[256] = "";
								strcat(sql, "SELECT ID, TITLE, AUTHOR, DATE FROM POSTLIST WHERE BOARDNAME='");
								strcat(sql, board_name);
								strcat(sql, "';");
								rc = sqlite3_exec(db, sql, list, "0", &zErrMsg);
							}							
						}
						else
						{
							char key[256] = "";
							int index = 0;
							for(int j=i+3;i<strlen(buffer);j++)
							{
								if(buffer[j] == '\r' || buffer[j] == '\n')
								{
									break;
								}
								key[index] = buffer[j];
								index++;
							}
							key[index] = '\0';
							char *message = "\tID\t\t\tTitle\t\t\tAuthor\t\t\tDate\n";
							send(sd, message, strlen(message), 0);
							char sql[256] = "";
							strcat(sql, "PRAGMA case_sensitive_like=ON;SELECT ID, TITLE, AUTHOR, DATE FROM POSTLIST WHERE BOARDNAME='");
							strcat(sql, board_name);
							strcat(sql, "' AND TITLE LIKE '%");
							strcat(sql, key);
							strcat(sql, "%';");
							rc = sqlite3_exec(db, sql, list, "0", &zErrMsg);
						}
					}
					else if(buffer[0] == 'r' && buffer[1] == 'e' && buffer[2] == 'a' && buffer[3] == 'd')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 1)
						{
							char *message = "Usage: read <post-id>\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							char post_id[256] = "";
							int i, index = 0;
							for(i=5;i<strlen(buffer);i++)
							{
								if(buffer[i] == '\r' || buffer[i] == '\n')
								{
									break;
								}
								post_id[index] = buffer[i];
								index++;
							}
							post_id[index] = '\0';
							char sql[256] = "";
							strcat(sql, "SELECT BOARDNAME FROM POSTLIST WHERE ID=");
							strcat(sql, post_id);
							rc = sqlite3_exec(db, sql, get_id, "0", &zErrMsg);
							if(result == 0)
							{
								char *message = "Post is not exist.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char sql2[256] = "";
								strcat(sql2, "SELECT AUTHOR, TITLE, DATE, CONTENT FROM '");
								strcat(sql2, id);
								strcat(sql2, "' WHERE ID='");
								strcat(sql2, post_id);
								strcat(sql2, "';");
								rc = sqlite3_exec(db, sql2, list_post, "0", &zErrMsg);
								char sql3[256] = "";
								strcat(sql3, "SELECT * FROM COMMENTLIST WHERE POSTID='");
								strcat(sql3, post_id);
								strcat(sql3, "';");
								rc = sqlite3_exec(db, sql3, list_comment, "0", &zErrMsg);
							}
							
						}
					}
					else if(buffer[0] == 'd' && buffer[1] == 'e' && buffer[2] == 'l' && buffer[3] == 'e' && buffer[4] == 't' && buffer[5] == 'e' && buffer[6] == '-' && buffer[7] == 'p' && buffer[8] == 'o' && buffer[9] == 's' && buffer[10] == 't')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 1)
						{
							char *message = "Usage: delete-post <post-id>\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							if(strlen(login_user[socket_index].username) == 0)
							{
								char *message = "Please login first.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char post_id[256] = "";
								int i, index = 0;
								for(i=12;i<strlen(buffer);i++)
								{
									if(buffer[i] == '\r' || buffer[i] == '\n')
									{
										break;
									}
									post_id[index] = buffer[i];
									index++;
								}
								post_id[index] = '\0';
								char sql[256] = "";
								strcat(sql, "SELECT BOARDNAME FROM POSTLIST WHERE ID=");
								strcat(sql, post_id);
								rc = sqlite3_exec(db, sql, get_id, "0", &zErrMsg);
								if(result == 0)
								{
									char *message = "Post is not exist.\n";
									send(sd, message, strlen(message), 0);
								}
								else
								{
									char sql[256] = "";
									strcat(sql, "SELECT AUTHOR FROM POSTLIST WHERE ID=");
									strcat(sql, post_id);
									rc = sqlite3_exec(db, sql, get_id, "0", &zErrMsg);
									if(strcmp(id, login_user[socket_index].username))
									{
										char *message = "Not the post owner.\n";
										send(sd, message, strlen(message), 0);
									}
									else
									{
										char sql4[256] = "";
										strcat(sql4, "SELECT BOARDNAME FROM POSTLIST WHERE ID=");
										strcat(sql4, post_id);
										rc = sqlite3_exec(db, sql4, get_id, "0", &zErrMsg);
										char sql[256] = "";
										strcat(sql, "DELETE FROM '");
										strcat(sql, id);
										strcat(sql, "' WHERE ID='");
										strcat(sql, post_id);
										strcat(sql, "';");
										rc = sqlite3_exec(db, sql, callback, "0", &zErrMsg);
										char sql2[256] = "";
										strcat(sql2, "DELETE FROM POSTLIST WHERE ID='");
										strcat(sql2, post_id);
										strcat(sql2, "';");
										rc = sqlite3_exec(db, sql2, callback, "0", &zErrMsg);
										char sql3[256] = "";
										strcat(sql2, "DELETE FROM COMMENTLIST WHERE POSTID='");
										strcat(sql2, post_id);
										strcat(sql2, "';");
										rc = sqlite3_exec(db, sql3, callback, "0", &zErrMsg);
										char *message = "Delete successfully.\n";
										send(sd, message, strlen(message), 0);
									}
								}
							}
							
							
						}
					}
					else if(buffer[0] == 'u' && buffer[1] == 'p' && buffer[2] == 'd' && buffer[3] == 'a' && buffer[4] == 't' && buffer[5] == 'e' && buffer[6] == '-' && buffer[7] == 'p' && buffer[8] == 'o' && buffer[9] == 's' && buffer[10] == 't')
					{
						
						if(strlen(login_user[socket_index].username) == 0)
						{
							char *message = "Please login first.\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							char post_id[256] = "", new_title[256]="", new_content[256]="";
							int i, j, index = 0;
							for(i=12;i<strlen(buffer);i++)
							{
								if(buffer[i] == ' ')
								{
									break;
								}
								post_id[index] = buffer[i];
								index++;
							}
							post_id[index] = '\0';
							index = 0;
							if(buffer[i+3] == 't')
							{
								for(j=i+9;j<strlen(buffer);j++)
								{
									if(buffer[j] == '\r' || buffer[j] == '\n')
									{
										break;
									}
									new_title[index] = buffer[j];
									index++;
								}
								new_title[index] = '\0';
								char sql[256] = "";
								strcat(sql, "SELECT BOARDNAME FROM POSTLIST WHERE ID=");
								strcat(sql, post_id);
								rc = sqlite3_exec(db, sql, get_id, "0", &zErrMsg);
								if(result == 0)
								{
									char *message = "Post is not exist.\n";
									send(sd, message, strlen(message), 0);
								}
								else
								{
									char sql[256] = "";
									strcat(sql, "SELECT AUTHOR FROM POSTLIST WHERE ID=");
									strcat(sql, post_id);
									rc = sqlite3_exec(db, sql, get_id, "0", &zErrMsg);
									if(strcmp(id, login_user[socket_index].username))
									{
										char *message = "Not the post owner.\n";
										send(sd, message, strlen(message), 0);
									}
									else
									{
										char sql3[256] = "";
										strcat(sql3, "SELECT BOARDNAME FROM POSTLIST WHERE ID=");
										strcat(sql3, post_id);
										rc = sqlite3_exec(db, sql3, get_id, "0", &zErrMsg);
										char sql[256] = "";
										strcat(sql, "UPDATE POSTLIST SET TITLE='");
										strcat(sql, new_title);
										strcat(sql, "' WHERE ID='");
										strcat(sql, post_id);
										strcat(sql, "';");
										rc = sqlite3_exec(db, sql, callback, "0", &zErrMsg);
										char sql2[256] = "";
										strcat(sql2, "UPDATE '");
										strcat(sql2, id);
										strcat(sql2, "' SET TITLE='");
										strcat(sql2, new_title);
										strcat(sql2, "' WHERE ID='");
										strcat(sql2, post_id);
										strcat(sql2, "';");
										rc = sqlite3_exec(db, sql2, callback, "0", &zErrMsg);
										char *message = "Update successfully.\n";
										send(sd, message, strlen(message), 0);
									}
								}
							}
							if(buffer[i+3] == 'c')
							{
								for(j=i+11;j<strlen(buffer);j++)
								{
									if(buffer[j] == '\r' || buffer[j] == '\n')
									{
										break;
									}
									new_content[index] = buffer[j];
									index++;
								}
								new_content[index] = '\0';
								char sql[256] = "";
								strcat(sql, "SELECT BOARDNAME FROM POSTLIST WHERE ID=");
								strcat(sql, post_id);
								rc = sqlite3_exec(db, sql, get_id, "0", &zErrMsg);
								if(result == 0)
								{
									char *message = "Post is not exist.\n";
									send(sd, message, strlen(message), 0);
								}
								else
								{
									char sql[256] = "";
									strcat(sql, "SELECT AUTHOR FROM POSTLIST WHERE ID=");
									strcat(sql, post_id);
									rc = sqlite3_exec(db, sql, get_id, "0", &zErrMsg);
									if(strcmp(id, login_user[socket_index].username))
									{
										char *message = "Not the post owner.\n";
										send(sd, message, strlen(message), 0);
									}
									else
									{
										char sql3[256] = "";
										strcat(sql3, "SELECT BOARDNAME FROM POSTLIST WHERE ID=");
										strcat(sql3, post_id);
										rc = sqlite3_exec(db, sql3, get_id, "0", &zErrMsg);
										char sql2[256] = "";
										strcat(sql2, "UPDATE '");
										strcat(sql2, id);
										strcat(sql2, "' SET CONTENT='");
										strcat(sql2, new_content);
										strcat(sql2, "' WHERE ID='");
										strcat(sql2, post_id);
										strcat(sql2, "';");
										rc = sqlite3_exec(db, sql2, callback, "0", &zErrMsg);
										char *message = "Update successfully.\n";
										send(sd, message, strlen(message), 0);
									}
								}
							}
						}
					}
					else if(buffer[0] == 'c' && buffer[1] == 'o' && buffer[2] == 'm' && buffer[3] == 'm' && buffer[4] == 'e' && buffer[5] == 'n' && buffer[6] == 't')
					{
						if(strlen(login_user[socket_index].username) == 0)
						{
							char *message = "Please login first.\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							char post_id[256] = "", comment[256] = "";
							int i, index = 0;
							for(i=8;i<strlen(buffer);i++)
							{
								if(buffer[i] == ' ')
								{
									break;
								}
								post_id[index] = buffer[i];
								index++;
							}
							post_id[index] = '\0';
							index = 0;
							for(int j=i+1;j<strlen(buffer);j++)
							{
								if(buffer[j] == '\r' || buffer[j] == '\n')
								{
									break;
								}
								comment[index] = buffer[j];
								index++;
							}
							comment[index] = '\0';
							char sql[256] = "";
							strcat(sql, "SELECT BOARDNAME FROM POSTLIST WHERE ID=");
							strcat(sql, post_id);
							rc = sqlite3_exec(db, sql, get_id, "0", &zErrMsg);
							if(result == 0)
							{
								char *message = "Post is not exist.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char sql[256] = "";
								strcat(sql, "INSERT INTO COMMENTLIST (POSTID, COMMENT, USERNAME) VALUES ('");
								strcat(sql, post_id);
								strcat(sql, "', '");
								strcat(sql, comment);
								strcat(sql, "', '");
								strcat(sql, login_user[socket_index].username);
								strcat(sql, "'); ");
								rc = sqlite3_exec(db, sql, callback, "0", &zErrMsg);
								char *message = "Comment successfully.\n";
								send(sd, message, strlen(message), 0);
							}
						}
					}
					else
					{
						char *message = "Wrong command\n" \
										"Usage: register <username> <email> <password>\n" \
										"       login <username> <password>\n" \
										"       logout\n" \
										"       whoami\n" \
										"       exit\n";
						send(sd, message, strlen(message), 0);
					}
				}
				char *prompt = "% ";
				send(sd, prompt, strlen(prompt), 0);
			}
		}
	}
	sqlite3_close(db);
}