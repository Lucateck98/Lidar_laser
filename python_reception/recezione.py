import requests
import math
import time
from datetime import datetime
import pandas as pd
import pyglet
window_x =1500  
window_y =900
URL ="http://192.168.1.131/"
zoomLidar = 15 # maggiore il numero, piu grande è la visualizzazione
distanceLidar = 10


def read_html_esp():#Nested Array 0,1
    try:
        response = requests.get(URL,timeout=1)
    except :
        return []

    else:
        listofvalues = ((response.text).split("\r\n")[:-2])
        data = [(x.split(" ")) for x in listofvalues]
        print(data)

        #data.sort(key=lambda x : x[0])
        return data

class Window(pyglet.window.Window):
    def __init__(self, refreshrate):
        super().__init__(window_x,window_y,"Radar")
        self.refreshrate = refreshrate
        self.alive = 1
        self.drawing = []
        self.batch = pyglet.graphics.Batch()
        self.origin_x = int(self.width/2)
        self.origin_y = int(self.height/2)
        self.a = True
        self.time_start = time.time()
        self.time_end = self.time_start+15


    def on_close(self):
        self.alive = 0


    def run(self):
        while self.alive :
            self.render()
            if(time.time()> self.time_end):
                self.alive = False

    def drawCircle(self,data):
        centerpoint = pyglet.shapes.Circle(self.origin_x, self.origin_y, 10, color=(255, 0, 0), batch=self.batch)
        self.drawing.append(centerpoint)
        #print(time.time_ns()/1000000,"before vector")
        if (0) :
            for circle in data:
                vector = pyglet.math.Vec2.from_polar(int(circle[1])/2, math.radians(float(circle[0])/100))#.clamp(-800, 800)
                self.drawing.append(pyglet.shapes.Circle(-vector.x+self.origin_x, vector.y+self.origin_y, 2, batch=self.batch))
        #print(time.time_ns()/1000000,"after vector")
        #data[0] = angolo °
        #data[1] = distanza : mm
        vector_1 = pyglet.math.Vec2(0,0)
        vector_2 = pyglet.math.Vec2(0,0)
        if (1) :
            for i in range(len(data)-1) :
                if ( vector_2.x == 0 ) :
                    vector_1 = pyglet.math.Vec2.from_polar(int(data[i][1])/zoomLidar, math.radians(float(data[i][0])/100))
                    vector_2= pyglet.math.Vec2.from_polar(int(data[i+1][1])/zoomLidar, math.radians(float(data[i+1][0])/100))
                else:
                    vector_1 = vector_2
                    vector_2= pyglet.math.Vec2.from_polar(int(data[i+1][1])/zoomLidar, math.radians(float(data[i+1][0])/100))
                if (vector_1.distance(vector_2) <= distanceLidar):
                    self.drawing.append(pyglet.shapes.Line(-vector_1.x + self.origin_x,vector_1.y + self.origin_y,-vector_2.x + self.origin_x,vector_2.y + self.origin_y,batch=self.batch))

                
        self.clear()
        self.batch.draw()
        

    def render(self):
        data = read_html_esp()
        self.drawing = []
        self.dispatch_events()
        if len(data) != 0 :
            self.drawCircle(data)
        self.flip()

if __name__== "__main__":
    radar = Window(60)
    #time.sleep(5)
    radar.run()
  