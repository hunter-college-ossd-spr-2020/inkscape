/*
 *  RasterFont.cpp
 *  testICU
 *
 */

#include "RasterFont.h"

#include <livarot/Ligne.h>
#include <livarot/Path.h>
#include <livarot/Shape.h>
#include <libnr/nr-point.h>
#include <libnr/nr-point-ops.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-pixblock.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/raster-glyph.h>
#include <libnrtype/raster-position.h>


static void glyph_run_A8_OR (raster_info &dest,void */*data*/,int st,float vst,int en,float ven);

void          font_style::Apply(Path* src,Shape* dest) {
	src->Convert(0.25);
	if ( stroke_width > 0 ) {
		if ( nbDash > 0 ) {
			double dlen = 0.0;
			const float scale = 1/*NR_MATRIX_DF_EXPANSION (&transform)*/;
			for (int i = 0; i < nbDash; i++)  dlen += dashes[i] * scale;
			if (dlen >= 0.01) {
				float   sc_offset = dash_offset * scale;
				float   *tdashs=(float*)malloc((nbDash+1)*sizeof(float));
				while ( sc_offset >= dlen ) sc_offset-=dlen;
				tdashs[0]=dashes[0] * scale;
				for (int i=1;i<nbDash;i++) tdashs[i]=tdashs[i-1]+dashes[i] * scale;
				src->DashPolyline(0.0,0.0,dlen,nbDash,tdashs,true,sc_offset);
				free(tdashs);
			}
		}
		src->Stroke(dest,false,0.5*stroke_width,stroke_join,stroke_cap,0.5*1.414*stroke_width);
	} else {
		src->Fill(dest,0);
	}
}

raster_font::raster_font(void)
{
//  printf("raster font born\n");

  refCount=0;
  daddy=NULL;
	
	style.transform=NR::Matrix();
	style.stroke_width=0;
	style.stroke_cap=butt_straight;
	style.stroke_join=join_straight;
	style.nbDash=0;
	style.dash_offset=0;
	style.dashes=NULL;
  
  nbBase=maxBase=0;
  bases=NULL;
}
raster_font::~raster_font(void)
{
//  printf("raster font death\n");
	if ( daddy ) daddy->RemoveRasterFont(this);
	daddy=NULL;
  if ( style.dashes ) free(style.dashes);
	for (int i=0;i<nbBase;i++) delete bases[i];
  if ( bases ) free(bases);
}
  
void           raster_font::Unref(void)
{
	refCount--;
//  printf("raster %x unref'd %i\n",this,refCount);
	if ( refCount <= 0 ) {
		if ( daddy ) daddy->RemoveRasterFont(this);
		daddy=NULL;
		delete this;
	}
}
void           raster_font::Ref(void)
{
	refCount++;
//  printf("raster %x ref'd %i\n",this,refCount);
}
raster_glyph*  raster_font::GetGlyph(int glyph_id)
{
  raster_glyph *res=NULL;
  if ( glyph_id_to_raster_glyph_no.find(glyph_id) == glyph_id_to_raster_glyph_no.end() ) {
    LoadRasterGlyph(glyph_id);
    if ( glyph_id_to_raster_glyph_no.find(glyph_id) == glyph_id_to_raster_glyph_no.end() ) { // recheck
    } else {
      res=bases[glyph_id_to_raster_glyph_no[glyph_id]];
		}
  } else {
    res=bases[glyph_id_to_raster_glyph_no[glyph_id]];
  }
  return res;
}
NR::Point      raster_font::Advance(int glyph_id)
{
	if ( daddy == NULL ) return NR::Point(0,0);
	double    a=daddy->Advance(glyph_id,style.vertical);
	NR::Point f_a=(style.vertical)?NR::Point(0,a):NR::Point(a,0);
	return f_a*style.transform;
}
void           raster_font::BBox(int glyph_id,NRRect *area)
{
	area->x0=area->y0=area->x1=area->y1=0;
	if ( daddy == NULL ) return;
	NR::Rect  res=daddy->BBox(glyph_id);
	NR::Point bmi=res.min(),bma=res.max();
	NR::Point tlp(bmi[0],bmi[1]),trp(bma[0],bmi[1]),blp(bmi[0],bma[1]),brp(bma[0],bma[1]);
	tlp=tlp*style.transform;
	trp=trp*style.transform;
	blp=blp*style.transform;
	brp=brp*style.transform;
	res=NR::Rect(tlp,trp);
	res.expandTo(blp);
	res.expandTo(brp);
	area->x0=(res.min())[0];
	area->y0=(res.min())[1];
	area->x1=(res.max())[0];
	area->y1=(res.max())[1];
}

void           raster_font::LoadRasterGlyph(int glyph_id)
{
  raster_glyph *res=NULL;
  if ( glyph_id_to_raster_glyph_no.find(glyph_id) == glyph_id_to_raster_glyph_no.end() ) {
    res=new raster_glyph();
    res->daddy=this;
    res->glyph_id=glyph_id;
    if ( nbBase >= maxBase ) {
      maxBase=2*nbBase+1;
      bases=(raster_glyph**)realloc(bases,maxBase*sizeof(raster_glyph*));
    }
    bases[nbBase]=res;
    glyph_id_to_raster_glyph_no[glyph_id]=nbBase;
    nbBase++;
  } else {
    res=bases[glyph_id_to_raster_glyph_no[glyph_id]];
  }
  if ( res == NULL ) return;
  if ( res->polygon ) return;
  if ( res->outline == NULL ) {
    if ( daddy == NULL ) return;
    Path*   outline=daddy->Outline(glyph_id,NULL);
    res->outline=new Path;
    if ( outline ) {
	res->outline->Copy(outline);
    }
    res->outline->Transform(style.transform);
  }
  Shape* temp=new Shape;
  res->polygon=new Shape;
	style.Apply(res->outline,temp);
  if ( style.stroke_width > 0 ) {
    res->polygon->ConvertToShape(temp,fill_nonZero);
  } else {
    res->polygon->ConvertToShape(temp,fill_oddEven);
  }
  delete temp;
	
	res->SetSubPixelPositionning(4);
}
void           raster_font::RemoveRasterGlyph(raster_glyph* who)
{
  if ( who == NULL ) return;
  int glyph_id=who->glyph_id;
  if ( glyph_id_to_raster_glyph_no.find(glyph_id) == glyph_id_to_raster_glyph_no.end() ) {
    int no=glyph_id_to_raster_glyph_no[glyph_id];
    if ( no >= nbBase-1 ) {
    } else {
      bases[no]=bases[--nbBase];
      glyph_id_to_raster_glyph_no[bases[no]->glyph_id]=no;
    }
    glyph_id_to_raster_glyph_no.erase(glyph_id_to_raster_glyph_no.find(glyph_id));
  } else {
    // not here
  }
}

/*int               top,bottom; // baseline is y=0
  int*              run_on_line; // array of size (bottom-top+1): run_on_line[i] gives the number of runs on line top+i
  int               nbRun;
  float_ligne_run*  runs;*/
  
raster_position::raster_position(void)
{
  top=0;
  bottom=-1;
  run_on_line=NULL;
  nbRun=0;
  runs=NULL;
}
raster_position::~raster_position(void)
{
  if ( run_on_line ) free(run_on_line);
  if ( runs ) free(runs);
}
  
void      raster_position::AppendRuns(int add,float_ligne_run* src,int y)
{
  if ( top > bottom ) {
    top=bottom=y;
    if ( run_on_line ) free(run_on_line);
    run_on_line=(int*)malloc(sizeof(int));
    run_on_line[0]=0;
  } else {
    if ( y < top ) {
 //     printf("wtf?\n");
      return;
    } else if ( y > bottom ) {
      int ob=bottom;
      bottom=y;
      run_on_line=(int*)realloc(run_on_line,(bottom-top+1)*sizeof(int));
      for (int i=ob+1;i<=bottom;i++) run_on_line[i-top]=0;
    }
  }
  
  if ( add <= 0 ) {
  } else {
    run_on_line[y-top]=add;
    runs=(float_ligne_run*)realloc(runs,(nbRun+add)*sizeof(float_ligne_run));
    memcpy(runs+nbRun,src,add*sizeof(float_ligne_run));
    nbRun+=add;
  }
}
void      raster_position::Blit(float ph,int pv,NRPixBlock &over)
{
  int   base_y=top+pv;
  int   first_y=top+pv,last_y=bottom+pv;
  if ( first_y < over.area.y0 ) first_y=over.area.y0;
  if ( last_y >= over.area.y1 ) last_y=over.area.y1-1;
  if ( first_y > last_y ) return;
  IntLigne  *theIL=new IntLigne();
  FloatLigne  *theI=new FloatLigne();
  
  char* mdata=(char*)over.data.px;
  if ( over.size == NR_PIXBLOCK_SIZE_TINY ) mdata=(char*)over.data.p;

  for (int y=first_y;y<=last_y;y++) {
    int   first_r=0,last_r=0;
    for (int i=base_y;i<y;i++) first_r+=run_on_line[i-base_y];
    last_r=first_r+run_on_line[y-base_y]-1;
    if ( first_r <= last_r ) {
			theI->Reset();
			for (int i=first_r;i<=last_r;i++) theI->AddRun(runs[i].st+ph,runs[i].en+ph,runs[i].vst,runs[i].ven,runs[i].pente);
//      for (int i=first_r;i<=last_r;i++) {runs[i].st+=ph;runs[i].en+=ph;}
//      theI->nbRun=theI->maxRun=last_r-first_r+1;
//      theI->runs=runs+first_r;
      
      theIL->Copy(theI);
      raster_info  dest;
      dest.startPix=over.area.x0;
      dest.endPix=over.area.x1;
      dest.sth=over.area.x0;
      dest.stv=y;
      dest.buffer=((uint32_t*)(mdata+(over.rs*(y-over.area.y0))));
      theIL->Raster(dest,NULL,glyph_run_A8_OR);
            
//      theI->nbRun=theI->maxRun=0;
//      theI->runs=NULL;
//      for (int i=first_r;i<=last_r;i++) {runs[i].st-=ph;runs[i].en-=ph;}
    }
  }
  delete theIL;
  delete theI;
}


/*  raster_font*      daddy;
  int               glyph_id;
  
  Path*             outline; 
  Shape*            polygon;
  
  int               nb_sub_pixel;
  raster_position*  sub_pixel;*/
  
raster_glyph::raster_glyph(void)
{
  daddy=NULL;
  glyph_id=0;
  outline=NULL;
  polygon=NULL;
  nb_sub_pixel=0;
  sub_pixel=NULL;
}
raster_glyph::~raster_glyph(void)
{
  if ( outline ) delete outline;
  if ( polygon ) delete polygon;
  if ( sub_pixel ) delete [] sub_pixel;
}


void      raster_glyph::SetSubPixelPositionning(int nb_pos)
{
  nb_sub_pixel=nb_pos;
  if ( nb_sub_pixel <= 0 ) nb_sub_pixel=0;
  if ( sub_pixel ) delete [] sub_pixel;
  sub_pixel=NULL;
  if ( nb_sub_pixel > 0 ) {
    sub_pixel=new raster_position[nb_pos];
    if ( polygon ) {
      for (int i=0;i<nb_sub_pixel;i++) LoadSubPixelPosition(i);
    }
  }
}
void      raster_glyph::LoadSubPixelPosition(int no)
{
  if ( no < 0 || no >= nb_sub_pixel ) return;
  if ( sub_pixel[no].top <= sub_pixel[no].bottom ) return;
  if ( polygon == NULL ) {
    if ( daddy == NULL ) return;
    daddy->LoadRasterGlyph(glyph_id);
    if ( polygon == NULL ) return;
  }
  
  float sub_delta=((float)no)/((float)nb_sub_pixel);
  
  polygon->CalcBBox();

  float  l=polygon->leftX,r=polygon->rightX,t=polygon->topY,b=polygon->bottomY;
  int    il,ir,it,ib;
  il=(int)floorf(l);
  ir=(int)ceilf(r);
  it=(int)floorf(t);
  ib=(int)ceilf(b);
  
  // version par FloatLigne
  int    curPt;
  float  curY;
  polygon->BeginQuickRaster(curY,curPt,1.0);
  
  FloatLigne* theI=new FloatLigne();
  
  polygon->DirectQuickScan(curY,curPt,(float)(it-1)+sub_delta,true,1.0);
  
  for (int y=it-1;y<ib;y++) {
    theI->Reset();
    if ( y&0x00000003 ) {
	    polygon->QuickScan(curY,curPt,((float)(y+1))+sub_delta,theI,false,1.0);
    } else {
	    polygon->QuickScan(curY,curPt,((float)(y+1))+sub_delta,theI,true,1.0);
    }
    theI->Flatten();
    
    sub_pixel[no].AppendRuns(theI->nbRun,theI->runs,y);
  }
  polygon->EndQuickRaster();
  delete theI;
}

void      raster_glyph::Blit(const NR::Point &at,NRPixBlock &over)
{
  if ( nb_sub_pixel <= 0 ) return;
	NR::Point   nat=at;
  nat[1]*=nb_sub_pixel;
  int pv=(int)round(nat[1]);
  int no=pv%nb_sub_pixel;
  if ( no < 0 ) no=-no;
  sub_pixel[no].Blit(nat[0],pv/nb_sub_pixel,over);
}



static void
glyph_run_A8_OR (raster_info &dest,void */*data*/,int st,float vst,int en,float ven)
{
  if ( st >= en ) return;
  if ( vst < 0 ) vst=0;
  if ( vst > 1 ) vst=1;
  if ( ven < 0 ) ven=0;
  if ( ven > 1 ) ven=1;
  float   sv=vst;
  float   dv=ven-vst;
  int     len=en-st;
  unsigned char*   d=(unsigned char*)dest.buffer;
  d+=(st-dest.startPix);
  if ( fabsf(dv) < 0.001 ) {
    if ( vst > 0.999 ) {
	    /* Simple copy */
	    while (len > 0) {
        d[0] = 255;
        d += 1;
        len -= 1;
	    }
    } else {
	    sv*=256;
	    unsigned int c0_24=(int)sv;
	    c0_24&=0xFF;
	    while (len > 0) {
        unsigned int da;
        /* Draw */
        da = 65025 - (255 - c0_24) * (255 - d[0]);
        d[0] = (da + 127) / 255;
        d += 1;
        len -= 1;
	    }
    }
  } else {
    if ( en <= st+1 ) {
	    sv=0.5*(vst+ven);
	    sv*=256;
	    unsigned int c0_24=(int)sv;
	    c0_24&=0xFF;
	    unsigned int da;
	    /* Draw */
	    da = 65025 - (255 - c0_24) * (255 - d[0]);
	    d[0] = (da + 127) / 255;
    } else {
	    dv/=len;
	    sv+=0.5*dv; // correction trapezoidale
	    sv*=16777216;
	    dv*=16777216;
	    int c0_24 = static_cast<int>(CLAMP(sv, 0, 16777216));
	    int s0_24 = static_cast<int>(dv);
	    while (len > 0) {
        unsigned int ca, da;
        /* Draw */
        ca = c0_24 >> 16;
        if ( ca > 255 ) ca=255;
        da = 65025 - (255 - ca) * (255 - d[0]);
        d[0] = (da + 127) / 255;
        d += 1;
        c0_24 += s0_24;
        c0_24 = CLAMP (c0_24, 0, 16777216);
        len -= 1;
	    }
    }
  }
}
