import sys
from socket import *
from time import ctime
import random
import string
import boto3
import time

s3 = boto3.resource('s3')
login_bucket_name = ''
HOST = str(sys.argv[1])
PORT = int(sys.argv[2])
BUFSIZ = 1024
ADDR = (HOST, PORT)
tcpCliSock = socket(AF_INET, SOCK_STREAM)
tcpCliSock.connect(ADDR)

"""WELCOME"""
data = tcpCliSock.recv(BUFSIZ)
print(data.decode('utf-8'), end = '')

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
		if response.decode('utf-8') == 'Register successfully.\n':
			bucket_name = ''.join(random.choice(string.ascii_lowercase) for i in range(50))
			s3.create_bucket(Bucket=bucket_name)
			command = 'bucket_name ' + bucket_name + ' ' + command.split(' ')[1]
			tcpCliSock.send(command.encode('utf-8'))
	"""LOGIN"""
	if command.split(' ')[0] == 'login':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8').split(' ')[0] == 'Welcome,':
			response = tcpCliSock.recv(BUFSIZ)
			login_bucket_name = response.decode('utf-8')
	"""LOGOUT""" #some bug
	if command.split(' ')[0] == 'logout':
		time.sleep(1)
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		# if response.decode('utf-8') == 'Bye, ':
		# 	print(tcpCliSock.recv(BUFSIZ).decode('utf-8'), end = '')
	"""WHOAMI"""
	if command.split(' ')[0] == 'whoami':
		time.sleep(1)
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		# if response.decode('utf-8') != 'Please login first.\n' and response.decode('utf-8') != 'Usage: whoami\n':
		# 	print(tcpCliSock.recv(BUFSIZ).decode('utf-8'), end = '')
	"""CREATE-BOARD"""
	if command.split(' ')[0] == 'create-board':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""LIST-BOARD"""
	if command.split(' ')[0] == 'list-board':
		time.sleep(1)
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		# response = tcpCliSock.recv(BUFSIZ)
		# if response.decode('utf-8') != 'done':
		# 	print(response.decode('utf-8'), end = '')
	"""LIST-POST"""
	if command.split(' ')[0] == 'list-post':
		time.sleep(1)
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		# if response.decode('utf-8') != 'Board is not exist.\n':
		# 	response = tcpCliSock.recv(BUFSIZ)
		# 	if response.decode('utf-8') != 'done':
		# 		print(response.decode('utf-8'), end = '')
	"""CREATE-POST"""
	if command.split(' ')[0] == 'create-post':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8') != 'Please login first.\n' and response.decode('utf-8') != 'Board does not exist.\n':
			filename = tcpCliSock.recv(BUFSIZ).decode('utf-8') + '.txt'
			content = command.split('--content ')[1]
			with open(filename, 'w') as f:
				f.write(content + 'ENDOFCONTENT ')
			target_bucket = s3.Bucket(login_bucket_name)
			target_bucket.upload_file(filename, filename)
	"""READ"""
	if command.split(' ')[0] == 'read':
		post_id = command.split(' ')[1]
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8') != 'Post does not exist.\n':
			time.sleep(1)
			filename = post_id + '.txt'
			bucket_name = tcpCliSock.recv(BUFSIZ).decode('utf-8')
			target_bucket = s3.Bucket(bucket_name)
			target_object = target_bucket.Object(filename)
			object_content = target_object.get()['Body'].read().decode()
			object_content = object_content.replace('<br>', '\n\t')
			post = object_content.split('ENDOFCONTENT')[0]
			comment = object_content.split('ENDOFCONTENT')[1]
			print('\t--\n\t', end = '')
			print(post)
			print('\t--\n', end = '')
			if len(comment) > 1:
				print(comment, end = '')
	"""DELETE-POST"""
	if command.split(' ')[0] == 'delete-post':
		post_id = command.split(' ')[1]
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8') == 'Delete successfully.\n':
			target_bucket = s3.Bucket(login_bucket_name)
			target_object = target_bucket.Object(post_id + '.txt')
			target_object.delete()
	"""UPDATE-POST"""
	if command.split(' ')[0] == 'update-post':
		post_id = command.split(' ')[1]
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8') == 'Update successfully.\n' and command.split(' ')[2] == '--content':
			filename = post_id + '.txt'
			target_bucket = s3.Bucket(login_bucket_name)
			target_object = target_bucket.Object(filename)
			object_content = target_object.get()['Body'].read().decode()
			target_object.delete()
			comment = object_content.split('ENDOFCONTENT')[1]
			post = command.split('--content ')[1]
			with open(filename, 'w') as f:
				if len(comment) > 1: 
					f.write(post + 'ENDOFCONTENT ' + comment)
				else:
					f.write(post + 'ENDOFCONTENT ')
			target_bucket.upload_file(filename, filename)
	"""COMMENT"""
	if command.split(' ')[0] == 'comment':
		post_id = command.split(' ')[1]
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8') == 'Comment successfully.\n':
			filename = post_id + '.txt'
			bucket_name = tcpCliSock.recv(BUFSIZ).decode('utf-8')
			username = tcpCliSock.recv(BUFSIZ).decode('utf-8')
			target_bucket = s3.Bucket(bucket_name)
			target_object = target_bucket.Object(filename)
			object_content = target_object.get()['Body'].read().decode()
			target_object.delete()
			post = object_content.split('ENDOFCONTENT')[0]
			comment = object_content.split('ENDOFCONTENT')[1]
			new_comment = command[len(post_id) + 9:]
			with open(filename, 'w') as f:
				if len(comment) > 1: 
					f.write(post + 'ENDOFCONTENT ' + comment + '\t' + username + ':' + new_comment + '\n')
				else:
					f.write(post + 'ENDOFCONTENT ' + '\t' + username + ':' + new_comment + '\n') 
			target_bucket.upload_file(filename, filename)
	"""MAIL-TO"""
	if command.split(' ')[0] == 'mail-to':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8') == 'Sent successfully.\n':
			response = tcpCliSock.recv(BUFSIZ).decode('utf-8')
			bucket_name = response.split(' ')[0]
			filename = response.split(' ')[1] + '.txt'
			content = command.split('--content ')[1]
			with open(filename, 'w') as f:
				f.write(content)
			target_bucket = s3.Bucket(bucket_name)
			target_bucket.upload_file(filename, filename)
	"""LIST-MAIL"""
	if command.split(' ')[0] == 'list-mail':
		time.sleep(1)
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
	"""RETR-MAIL"""
	if command.split(' ')[0] == 'retr-mail':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8') != 'Please login first.\n' and response.decode('utf-8') != 'No such mail.\n':
			time.sleep(1)
			filename = tcpCliSock.recv(BUFSIZ).decode('utf-8') + '.txt'
			target_bucket = s3.Bucket(login_bucket_name)
			target_object = target_bucket.Object(filename)
			object_content = target_object.get()['Body'].read().decode()
			object_content = object_content.replace('<br>', '\n\t')
			print('\t--\n\t', end = '')
			print(object_content)
			
	"""DELETE-MAIL"""
	if command.split(' ')[0] == 'delete-mail':
		response = tcpCliSock.recv(BUFSIZ)
		print(response.decode('utf-8'), end = '')
		if response.decode('utf-8') == 'Mail deleted.\n':
			filename = tcpCliSock.recv(BUFSIZ).decode('utf-8') + '.txt'
			target_bucket = s3.Bucket(login_bucket_name)
			target_object = target_bucket.Object(filename)
			target_object.delete()
tcpCliSock.close()