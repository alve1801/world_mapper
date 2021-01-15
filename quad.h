class Quad{public:
	Quad*children[4];
	// 01 y
	// 23
	// x
	// coords or scale?
	short col;

	Quad(short col=0xfff):col(col){for(int i=0;i<4;i++)children[i]=nullptr;}
	~Quad(){orphan();} // orphan does not delete current node

	void orphan(){for(int i=0;i<4;i++)if(children[i]!=nullptr){delete children[i]
	;children[i]=nullptr;}}char ch(){char a=0;for(int i=0;i<4;i++)if(children[i]
	!=nullptr)a++;return a;}bool leaf(){return ch()==0;}

	void render(short*buf,int sx,int sy,int ax,int ay,char s,bool debug=false){
		// buf,sx,sy are the screen buffer and sizes
		// ax ay are the screen coordinates we render ourselves at
		// scale tells us how big current quad is on screen (logarithmic)

		// adjust this to set how fine it draws stuff
		if(s<0 || ax>sx || ay>sy || ax+(1<<s)<0 || ay+(1<<s)<0)return;

		for(int x=max(ax);x<ax+(1<<s) && x<sx;x++)
		for(int y=max(ay);y<ay+(1<<s) && y<sy;y++){
		buf[x*sy+y]=col;
		if(debug)if(x==ax||x==ax+(1<<s)-1||y==ay||y==ay+(1<<s)-1)buf[x*sy+y]=0;
		}

		s--;
		if(children[0]!=nullptr)
			children[0]->render(buf,sx,sy,ax       ,ay       ,s,debug);
		if(children[1]!=nullptr)
			children[1]->render(buf,sx,sy,ax       ,ay+(1<<s),s,debug);
		if(children[2]!=nullptr)
			children[2]->render(buf,sx,sy,ax+(1<<s),ay       ,s,debug);
		if(children[3]!=nullptr)
			children[3]->render(buf,sx,sy,ax+(1<<s),ay+(1<<s),s,debug);
	}

	int set(int ax,int ay,char s,short color){ // could be a bool?
		// return value tells us whether parent needs to change color

		if(s<2){col=color;orphan();return 1;}

		char c=0;if(ax>(1<<s)){c+=2;ax-=1<<s;}if(ay>(1<<s)){c++;ay-=1<<s;}
		if(children[c]==nullptr)children[c]=new Quad(col);

		if(children[c]->set(ax,ay,s-1,color)){ // main part

			// if child is leaf and has same color as parent, delete it
			// calculate dominant color
			// if !=col
			//  set empty children to col
			//  set col to dominant
			//  remove all leafs w/ dominant color
			// if were leaf, return 1

			// we might want to recalculate dominant color even when we didnt reload

			if(children[c]->leaf() && children[c]->col==col){
				delete children[c];children[c]=nullptr;
			}

			// XXX rewrite this to account for new colorspace
			short maxcol[4096],maxindex=col;for(int i=0;i<4096;i++)maxcol[i]=0;
			for(int i=0;i<4;i++)
				if(children[i]!=nullptr)maxcol[children[i]->col]++;
				else maxcol[col]++;
			for(int i=0;i<4096;i++)if(maxcol[i]>maxcol[maxindex])maxindex=i;

			if(maxindex!=col){
				for(int i=0;i<4;i++)if(children[i]==nullptr)children[i]=new Quad(col);
				col=maxindex;
				for(int i=0;i<4;i++)if(children[i]!=nullptr)
					if(children[i]->col==maxindex && children[i]->leaf()){
						delete children[i];children[i]=nullptr;
					}
			}

			if(leaf())return 1;

		}

		return 0;

	}

	short get(int ax,int ay,char s){ // set but in reverse (and w/out the optimization)
		if(s==0)return col;
		char c=0;if(ax>(1<<s)){c+=2;ax-=1<<s;}if(ay>(1<<s)){c++;ay-=1<<s;}
		if(children[c]==nullptr)return col;
		return children[c]->get(ax,ay,s-1);
	}

	void bet(int ax,int ay,char s){ // get but for debugging
		if(s<1){printf(" %03x -\n",col);return;} // prolly not gonna happen, since we cant get the brush that small
		char c=0;if(ax>(1<<s)){c+=2;ax-=1<<s;}if(ay>(1<<s)){c++;ay-=1<<s;}
		printf("%i",c);
		if(children[c]==nullptr){printf(" %03x\n",col);return;}
		return children[c]->bet(ax,ay,s-1);
	}

	void save(FILE*f){
		// strictly speaking, the following can fit into the same byte
		putc(col&0xff,f);
		putc(col>>8,f);
		putc(
			(children[0]!=nullptr?1:0)|
			(children[1]!=nullptr?2:0)|
			(children[2]!=nullptr?4:0)|
			(children[3]!=nullptr?8:0),
			f
		);
		if(children[0]!=nullptr)children[0]->save(f);
		if(children[1]!=nullptr)children[1]->save(f);
		if(children[2]!=nullptr)children[2]->save(f);
		if(children[3]!=nullptr)children[3]->save(f);
	}

	void load(FILE*f){

		short palette[16]={
		  //0x000,0xfff,0xf00,0x0f0, 0x00f,0x0ff,0xf0f,0xff0
		  0x000,0x00a,0x0a0,0x0aa, 0xa00,0xa0a,0xa50,0xaaa,
		  0x555,0x55f,0x5f5,0x5ff, 0xf55,0xf5f,0xff5,0xfff
		};

		//col=palette[getc(f)];
		col=getc(f)|(getc(f)<<8);
		//col=col==0x3ca?0x8eb:col;

		char c=getc(f);
		if(c&1){children[0]=new Quad();children[0]->load(f);}
		if(c&2){children[1]=new Quad();children[1]->load(f);}
		if(c&4){children[2]=new Quad();children[2]->load(f);}
		if(c&8){children[3]=new Quad();children[3]->load(f);}
	}

	int depth(){
		if(leaf())return 0;
		int d=0,dt;
		for(int i=0;i<4;i++){
			dt=children[i]->depth();
			if(dt>d)d=dt;
		}
		return dt+1;
	}

	/*
	void export(){
		char filename[]="%i.ppm";
		for(int i=0;i<depth();i++){
			FILE*f=fopen(filename,"w");
			// print header - idk, figure out file size, should be some power of two
			// have buffer
			// render at that depth
			// print to file
			fclose(f)
		}
	}
	*/
};
