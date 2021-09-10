from kafka import KafkaProducer
from kafka import KafkaConsumer
import sys
from socket import *
from time import ctime
import random
import string
import time
import threading
thread = []
thread_count = 0
username = ''
HOST = str(sys.argv[1])
PORT = int(sys.argv[2])
BUFSIZ = 1024
ADDR = (HOST, PORT)
tcpCliSock = socket(AF_INET, SOCK_STREAM)
tcpCliSock.connect(ADDR)
subscribe_thing = []
"""WELCOME"""
data = tcpCliSock.recv(BUFSIZ)
print(data.decode('utf-8'), end = '')

def job(board_or_author, name, keywords):
	consumer = KafkaConsumer('111', group_id= username, bootstrap_servers= ['localhost:9092'])
	t = threading.currentThread()
	if board_or_author == 0:
		for msg in consumer:
			if not getattr(t, "do_run", True):
				break
			key = msg.key
			key = str(key, encoding='utf-8')
			if key.split('|')[0] == name:
				if key.split('|')[1].find(keywords) >= 0:
					print('Board:', key.split('|')[0],'Title:', key.split('|')[1], 'Author:', key.split('|')[2], '\n% ',end='')
	if board_or_author == 1:
		for msg in consumer:
			if not getattr(t, "do_run", True):
				break
			key = msg.key
			key = str(key, encoding='utf-8')
			if key.split('|')[2] == name:
				if key.split('|')[1].find(keywords) >= 0:
					print('Board:', key.split('|')[0],'Title:', key.split('|')[1], 'Author:', key.split('|')[2], '\n% ',end='')

while True:
	"""PROMPT"""
	print('% ', end ='')
	"""SEND COMMAND"""
	command = input()
	tcpCliSock.send(command.encode('utf-8'))
	"""EXIT"""
	if command.split(' ')[0] == 'exit':
		break
	"""REGISTER"""
	if command.split(' ')[0] == 'register':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""LOGIN"""
	if command.split(' ')[0] == 'login':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8').split(' ')[0] == 'Welcome,':
			username = command.split(' ')[1]
	"""LOGOUT"""
	if command.split(' ')[0] == 'logout':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8').split(' ')[0] == 'Bye,':
			username = ''
	"""WHOAMI"""
	if command.split(' ')[0] == 'whoami':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""CREATE-BOARD"""
	if command.split(' ')[0] == 'create-board':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""LIST-BOARD"""
	if command.split(' ')[0] == 'list-board':
		time.sleep(1)
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""LIST-POST"""
	if command.split(' ')[0] == 'list-post':
		time.sleep(1)
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""CREATE-POST"""
	if command.split(' ')[0] == 'create-post':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8').split(' ')[0] == 'Create':
			producer = KafkaProducer(bootstrap_servers=['localhost:9092'])
			title = command.split('--title ')[1].split(' --content')[0]
			key = command.split(' ')[1] + '|' + title + '|' + username
			key = bytes(key, encoding='utf8')
			future = producer.send('111', key=key, value=b'my_value', partition=0)
			future.get(timeout=1)
	"""READ"""
	if command.split(' ')[0] == 'read':
		time.sleep(1)
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""DELETE-POST"""
	if command.split(' ')[0] == 'delete-post':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""UPDATE-POST"""
	if command.split(' ')[0] == 'update-post':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""COMMENT"""
	if command.split(' ')[0] == 'comment':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""SUBSCRIBE"""
	if command.split(' ')[0] == 'subscribe':
		response = tcpCliSock.recv(BUFSIZ)
		if response.decode('utf-8') == 'ok':
			tmp = []
			match = 0
			if command.split(' ')[1] == '--board':
				for data in subscribe_thing:
					if data[0] == 'board' and command.split(' ')[2] == data[1] and command.split(' ')[4] == data[3]:
						match = 1
				if match == 0:
					tmp.append('board')
					tmp.append(command.split(' ')[2])
					tmp.append('key')
					tmp.append(command.split(' ')[4])
					tmp.append(thread_count)
					subscribe_thing.append(tmp)
					thread.append(threading.Thread(target = job, args = (0, command.split(' ')[2],command.split(' ')[4], )))
					thread[thread_count].start()
					thread_count += 1
					print('Subscribe successfully')
				if match == 1:
					print('Already subscribed')
			if command.split(' ')[1] == '--author':
				for data in subscribe_thing:
					if data[0] == 'author' and command.split(' ')[2] == data[1] and command.split(' ')[4] == data[3]:
						match = 1
				if match == 0:
					tmp.append('author')
					tmp.append(command.split(' ')[2])
					tmp.append('key')
					tmp.append(command.split(' ')[4])
					tmp.append(thread_count)
					subscribe_thing.append(tmp)
					thread.append(threading.Thread(target = job, args = (1, command.split(' ')[2],command.split(' ')[4], )))
					thread[thread_count].start()
					thread_count += 1
					print('Subscribe successfully')
				if match == 1:
					print('Already subscribed')
		else:
			print(response.decode('utf-8'), end='')
	"""UNSUBSCRIBE"""
	if command.split(' ')[0] == 'unsubscribe':
		response = tcpCliSock.recv(BUFSIZ)
		if response.decode('utf-8') == 'ok':
			tmp = []
			if command.split(' ')[1] == '--board':
				match = 0
				for data in subscribe_thing:
					if data[0] == 'board' and command.split(' ')[2] == data[1]:
						thread[data[4]].do_run = False
						match = 1
					else:
						tmp.append(data)
				if match == 1:
					print('Unsubscribe successfully')
				else:
					print('You haven\'t subscribed', command.split(' ')[2])
				subscribe_thing = tmp.copy()
			if command.split(' ')[1] == '--author':
				match = 0
				for data in subscribe_thing:
					if data[0] == 'author' and command.split(' ')[2] == data[1]:
						thread[data[4]].do_run = False
						match = 1
					else:
						tmp.append(data)
				if match == 1:
					print('Unsubscribe successfully')
				else:
					print('You haven\'t subscribed', command.split(' ')[2])
				subscribe_thing = tmp.copy()
		else:
			print(response.decode('utf-8'), end='')
	"""LIST-SUB"""
	if command.split(' ')[0] == 'list-sub':
		response = tcpCliSock.recv(BUFSIZ)
		if response.decode('utf-8') == 'ok':
			for data in subscribe_thing:
				print(data[0], ':', data[1], '  ',data[2], ':', data[3])
		else:
			print(response.decode('utf-8'), end='')
tcpCliSock.close()