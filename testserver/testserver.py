import socket
import struct
from StringIO import StringIO
import Tkinter as tk
import ImageTk
import Image
import thread
import threading
import Queue
import time

UDP_IP = "134.60.158.215"
UDP_PORT = 44044


def server():
	if not hasattr(server, "image_buffer"):
     		server.image_buffer = ""
	data, addr = sock.recvfrom(5000)
	global befehlthread	
	befehlthread.set_addr(addr) 
	payload = data[12:]
	pos = payload.find("\xff\xd8")
	if pos>-1 :
		server.image_buffer+=payload[:pos]
		try:
			img2 = ImageTk.PhotoImage(Image.open(StringIO(server.image_buffer)))
			panel.configure(image = img2)
			panel.image = img2
			print "frame ",len(server.image_buffer)
		except:
			print "some image error"
		server.image_buffer = ""
		server.image_buffer+=payload[pos:]
	else :
		server.image_buffer+=payload
	
        root.after(5, server)


class befehl (threading.Thread):

	def __init__(self,queu,lock,addr,stop):
		self.q = queu
		self.lock = lock
		self.addr = addr
		self.stop = stop
		threading.Thread.__init__(self)

	def run(self):
		while not self.stop.is_set():
			self.lock.acquire()
			if self.q.empty():
				startbyte = 255
				kamera = 1
				kick = 0
				motor_links = 0
				motor_rechts = 0
				checksum = 0
				befehl = struct.pack("Bbbbbb",startbyte,kamera,kick,motor_links,motor_rechts,checksum)
			else :	
				befehl = self.q.get()		
			send_number_bytes = sock.sendto(befehl, self.addr)
			self.lock.release()
			time.sleep(0.1)
	def set_addr(self,addr):
		self.addr=addr

def keydown(e,q):
	startbyte = 255
	kamera = 1
	kick = 0
	motor_links = 0
	motor_rechts = 0
	checksum = 0

	if e.char == "w":
		motor_links=50
		motor_rechts=50
	if e.char == "a":
		motor_links=-50
		motor_rechts=50
	if e.char == "s":
		motor_links=-50
		motor_rechts=-50
	if e.char == "d":
		motor_links=50
		motor_rechts=-50
	if e.char == "k":
		kick=1
	queueLock.acquire()
	if q.empty():
		q.put(struct.pack("Bbbbbb",startbyte,kamera,kick,motor_links,motor_rechts,checksum))
	queueLock.release()	



sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))
data, addr = sock.recvfrom(5000) 
	
	
befehlQueue = Queue.Queue(10)
queueLock = threading.Lock()
thread_stop= threading.Event()
befehlthread = befehl(befehlQueue,queueLock,addr,thread_stop)
befehlthread.start()

root = tk.Tk()
panel = tk.Label(root)
panel.pack(side = "bottom", fill = "both", expand = "yes")
root.bind("<KeyPress>", lambda event: keydown(event, befehlQueue))
root.after(100,server)
root.mainloop()

thread_stop.set()	
