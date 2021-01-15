#define OLC_PGE_APPLICATION
#include <stdlib.h>
#include <stdio.h> // mostly for debugging
#include "olc.h"
#define tilesize 12
#define sh 600
#define sw 500
#define ss 2

// god damn it c
int min(int a,int b=0){return(a<b?a:b);}
int max(int a,int b=0){return(a>b?a:b);}
int r(int a,int b=1,int c=0){return (a-c)/b*b+c;}

short palette[16]={ // XXX we dont need a palette anymore...
	//0x000,0xfff,0xf00,0x0f0, 0x00f,0x0ff,0xf0f,0xff0
	0x000,0x00a,0x0a0,0x0aa, 0xa00,0xa0a,0xa50,0xaaa,
	0x555,0x55f,0x5f5,0x5ff, 0xf55,0xf5f,0xff5,0xfff
};

#include "quad.h"

short newmap[sh*sw],oldmap[sh*sw],rin=0;
Quad m;
FILE*f;

int cox=tilesize,coy=0,bx,by; // screen offset and brush temps
unsigned char
scale=8, // screen scale
bs=0; // brush scale

bool debug=false,spraying=false;

void p(int x,int y){
	//if(x>0&&y>0&&x<(1<<scale)&&y<(1<<scale)) // XXX correct for brush scale
	//m.set(x>>bs,y>>bs,scale-bs-1,rin); // XXX round
	m.set(x>>bs,y>>bs,scale-bs-1,rin); // XXX round
}

void spray(int bx,int by){
	int brushsize=8,density=5; // XXX sliders
	brushsize=(8<<bs);
	for(int x=-brushsize;x<brushsize;x++)
	for(int y=-brushsize;y<brushsize;y++)
	if(x*x+y*y<brushsize*brushsize && rand()%100<density)
	p(x+bx,y+by);
}

class View:public olc::PixelGameEngine{
	public:View(){sAppName="MS Chungus";}
	bool OnUserCreate()override{return(render(0,0));}

	olc::Pixel sga(short color){int r=(color&0x0f00)<<4;
	int g=(color&0x00f0)<<8;int b=(color&0x000f)<<12;
	r=r|r>>4|r>>8|r>>12;g=g|g>>4|g>>8|g>>12;b=b|b>>4|b>>8|b>>12;
	return olc::Pixel(r,g,b);}

	bool render(int mx,int my){
		// clear newmap
		for(int x=0;x<sh;x++)for(int y=0;y<sw;y++)
		newmap[x*sw+y]=(x^y)&1?0x0444:0x0000;

		m.render(newmap,sh,sw,cox,coy,scale,debug); // istg

		// draw color picks
		for(int y=0;y<tilesize*8;y++){
			unsigned char ty=y*2/tilesize;
			int x=0;
			for(;x<tilesize  ;x++)newmap[x*sw+y]=rin;
			for(;x<tilesize*2;x++)newmap[x*sw+y]=ty<<8 | rin&0x0ff;
			for(;x<tilesize*3;x++)newmap[x*sw+y]=ty<<4 | rin&0xf0f;
			for(;x<tilesize*4;x++)newmap[x*sw+y]=ty<<0 | rin&0xff0;
			// "|rin" seems kinda unnecessary
		}

		// draw color selector bars
		int x=tilesize;
		for(;x<tilesize*2;x++){newmap[x*sw+((rin&0xf00)>>8)*tilesize/2]=0xfff;newmap[x*sw+(((rin&0xf00)>>8)+1)*tilesize/2-1]=0xfff;}
		for(;x<tilesize*3;x++){newmap[x*sw+((rin&0x0f0)>>4)*tilesize/2]=0xfff;newmap[x*sw+(((rin&0x0f0)>>4)+1)*tilesize/2-1]=0xfff;}
		for(;x<tilesize*4;x++){newmap[x*sw+((rin&0x00f)>>0)*tilesize/2]=0xfff;newmap[x*sw+(((rin&0x00f)>>0)+1)*tilesize/2-1]=0xfff;}

		// draw brush XXX account for bs and round
		// XXX AAAAAAAH
		mx=r(mx,1<<(bs+2),cox)-1;
		my=r(my,1<<(bs+2),coy)-1;
		if(mx>0 && my>0)
		for(int i=0;i<1<<(bs+2);i++){
		newmap[(mx  )*sw+my+i]=0x0f44;
		newmap[(mx+(4<<bs))*sw+my+i]=0x0f44;
		newmap[(mx+i)*sw+my  ]=0x0f44;
		newmap[(mx+i)*sw+my+(4<<bs)]=0x0f44;
		}

		// update screen
		for(int x=0;x<sh;x++)
		for(int y=0;y<sw;y++)
		if(newmap[x*sw+y]!=oldmap[x*sw+y]){
			oldmap[x*sw+y]=newmap[x*sw+y];
			Draw(x,y,sga(newmap[x*sw+y]));
		}

		return true;
	}

	bool OnUserUpdate(float fElapsedTime)override{
		int x=GetMouseX(),y=GetMouseY();
		//int lox=(x-tilesize*2)/(1<<scale)+cox,loy=y/(1<<scale)+coy;
		int lox=x-cox,loy=y-coy;

		if(GetMouse(0).bHeld){
			if(GetKey(olc::CTRL).bHeld){ // XXX and it is within bounds of canvas
			rin=m.get(lox,loy,scale-1);
			//putchar("\n");
			}
			else
			if(x>tilesize*4 || y>tilesize*8){
				if(spraying)spray(lox,loy);
				else p(lox,loy);
			}else{ // color pick
				//rin=y/tilesize;
				// also generally restructure

				// this code is terrible and i know it but its five in the morning rn so i aint gonna change it
				//char ty=(char)(16-((float)y-tilenum*tilesize)/(sw-tilenum*tilesize)*16);
				char ty=y*2/tilesize;
				     if(x<tilesize  ); // ignore
				else if(x<tilesize*2){rin = rin&0x0ff | ty<<8;} // select red
				else if(x<tilesize*3){rin = rin&0xf0f | ty<<4;} // select green
				else if(x<tilesize*4){rin = rin&0xff0 | ty<<0;} // select blue

			}

		}

		if(GetKey(olc::ESCAPE).bPressed || GetKey(olc::Q).bPressed)exit(0); // delete m?
		if(GetKey(olc::D).bPressed)debug=!debug; // display quads
		if(GetKey(olc::L).bPressed)cox=coy=1<<(scale-1); // recenter
		if(GetKey(olc::B).bPressed)spraying=false;
		if(GetKey(olc::S).bPressed)spraying=true;

		if(GetMouse(1).bHeld){m.bet(lox>>bs,loy>>bs,scale-bs-1);}

		if(GetKey(olc::O).bPressed){printf("loading save\n");f=fopen("save.cff","r");m.load(f);fclose(f);}
		if(GetKey(olc::M).bPressed){printf("saving file\n");f=fopen("save.cff","w");m.save(f);fclose(f);}

		// export

		// XXX include some maximum border offset
		char movspeed=2;
		if(GetKey(olc::SHIFT).bHeld)movspeed=5;
		if(GetKey(olc::CTRL ).bHeld)movspeed=15;
		if(GetKey(olc::UP   ).bHeld)coy+=movspeed;
		if(GetKey(olc::DOWN ).bHeld)coy-=movspeed;
		if(GetKey(olc::LEFT ).bHeld)cox+=movspeed;
		if(GetKey(olc::RIGHT).bHeld)cox-=movspeed;

		// XXX center after the following (coords += (effective_size)/2)
		// doesnt quite work
		//  it doesnt account for screen dimensions
		//  so, subtract screen size, offset, add screen size
		if(GetKey(olc::R).bPressed && scale<32){scale++;cox-=1<<(scale-2);coy-=1<<(scale-2);}
		if(GetKey(olc::F).bPressed && scale>5){scale--;cox+=1<<(scale-1);coy+=1<<(scale-1);}
		if(GetKey(olc::NP_ADD).bPressed && bs<4)bs++;
		if(GetKey(olc::NP_SUB).bPressed && bs>0)bs--;
		if(GetKey(olc::NP_MUL).bPressed){cox=coy=bs=0;scale=8;}


		return(render(x,y));
	}
};

int main(){View v;if(v.Construct(sh,sw,ss,ss))v.Start();return 0;}

/*

i think one of the problems w/ the brush display is that it isnt even clearly defined exactly where were drawing
	so, rounding errors in the setter... great.
	seems to be offset by about half a quad

correction: it appears theres a discreptancy between the levels we draw at and which we see
	and that is where the iffyness of the paintbrush comes

notes. we need notes.

*/
