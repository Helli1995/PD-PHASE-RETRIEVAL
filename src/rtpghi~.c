//  Created by maximilian helligrath on 28.07.23.
//

#include <m_pd.h>
#include <ltfat.h>

#if !defined(PD_FLOATSIZE)
  /* normally, our floats (t_float, t_sample,...) are 32bit */
# define PD_FLOATSIZE 32
#endif

#if PD_FLOATSIZE == 32
#define LTFAT_SINGLE

#elif PD_FLOATSIZE == 64
#define LTFAT_DOUBLE
#endif

#include <phaseret.h>

#define rtpghi_init PHASERET_NAME(rtpghi_init)
#define rtpghi_done PHASERET_NAME(rtpghi_done)
#define rtpghi_set_tol PHASERET_NAME(rtpghi_set_tol)
#define rtpghi_set_causal PHASERET_NAME(rtpghi_set_causal)
#define rtpghi_execute PHASERET_NAME(rtpghi_execute)
#define ltfat_complex LTFAT_COMPLEX
#define rtpghi_state PHASERET_NAME(rtpghi_state)


static t_class *rtpghi_tilde_class;
typedef struct _rtpghi_tilde {
	t_object  x_obj;
	t_float overlap;
	t_float tol_pd;
	t_float do_causal_pd;
	t_symbol *window_type_pd;
	rtpghi_state* sta_pd;
	ltfat_complex *c;
	ltfat_int blocksize;
    t_float f;
} t_rtpghi_tilde;

t_int *rtpghi_tilde_perform(t_int *w) {
	
	t_rtpghi_tilde *x = (t_rtpghi_tilde *)(w[1]);
	t_sample *s =      (t_sample *) (w[2]);
	t_sample *out=(t_sample *) (w[3]);
	t_sample *out1= (t_sample *) (w[4]);
	int            n =             (int)(w[5]);

	ltfat_complex *c = (ltfat_complex *) x->c;
	int e = 0;

	if ((s==NULL) || ((x->sta_pd)==NULL)) {
	   pd_error(x, "arrays not initialised");
	}
	
	else
	{
		e = rtpghi_execute(x->sta_pd, s, c);

		if (e == 0) {
			while (n--) {
				*out++ = creal(*c);
				*out1++ = cimag(*c++);
			}
		}
		else {
			pd_error(x, "error of type: %d", e);
		}
	}

 return (w+6);
	
}


void rtpghi_tilde_recreatestate(t_rtpghi_tilde*x, t_symbol window, int blocksize, int overlap) {
	if(x->sta_pd && (window.s_name != x->window_type_pd->s_name || blocksize != x->blocksize || overlap != x->overlap)) {
		rtpghi_done(&(x->sta_pd));
		x->sta_pd=NULL;
		post("sta zeroed");
	}
	x->blocksize = blocksize;
	x->overlap = overlap;
	double tol = x->tol_pd;
	int do_causal = x->do_causal_pd;
	*(x->window_type_pd)=window;
	LTFAT_FIRWIN win;
	double gamma;
	
	if (x->window_type_pd != NULL) {
		win = ltfat_str2firwin(x->window_type_pd->s_name);
		post("blib");
	}
	else {
		//const char win_[8] = "hanning";
		win = ltfat_str2firwin("hann");
		ltfat_int w = 1;
		gamma = phaseret_firwin2gamma(win, x->blocksize);
		if (rtpghi_init(w, blocksize/(x->overlap), x->blocksize, gamma, tol, do_causal, &(x->sta_pd))) {
			pd_error(x, "failed to init state");
			x->sta_pd=NULL;
			}
	}
		
	if (!x->sta_pd) {
		ltfat_int w = 1;
		gamma = phaseret_firwin2gamma(win, x->blocksize);
		if (rtpghi_init(w, blocksize/(x->overlap), x->blocksize, gamma, tol, do_causal, &(x->sta_pd))) {
			pd_error(x, "failed to init state");
			x->sta_pd=NULL;
			}
		}
}

void rtpghi_tilde_dsp(t_rtpghi_tilde *x, t_signal **sp) {
	//post("start DSP");
	ltfat_int M = (ltfat_int) sp[0]->s_n;
	if ((x->blocksize) == 0) {
		(x->blocksize) = M;
	}
	
	rtpghi_tilde_recreatestate(x, *(x->window_type_pd), x->blocksize, x->overlap);
	
	if (x->c != NULL) {
		freebytes(x->c, M * sizeof *(x->c));
		x->c = NULL;
	}
	if (x->c == NULL) {
		x->c = getbytes(M * sizeof *(x->c));
	}
	
   
   dsp_add(rtpghi_tilde_perform, 5,x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void rtpghi_tilde_causal(t_rtpghi_tilde *x, t_floatarg f) {
	
	if ((f == 0.0) || (f == 1.0)) {
		int causal = f;
		rtpghi_set_causal(x->sta_pd, causal);
	}
	else {
		pd_error(x, "failed to set (a)causal, input has to be either 0 or 1, but got: %f", f);
	}
}
void rtpghi_tilde_tol(t_rtpghi_tilde *x, t_floatarg f) {
	
	if ((f > 0.0) && (f < 1.0)) {
		double tol = f;
		rtpghi_set_tol(x->sta_pd, tol);
	}
	else {
		pd_error(x, "failed to set tol, input float has to be  > 0 && <1, but got: %f", f);
	}
}

void rtpghi_tilde_overlap(t_rtpghi_tilde *x, t_floatarg f) {
	if ((f > 0.0) && (f <= x->blocksize)) {
		rtpghi_tilde_recreatestate(x, *(x->window_type_pd), x->blocksize, f);
	}
	else {
		pd_error(x, "failed to set overlap, input float has to be  > 0 && <= blocksize, but got: %f", f);
	}
}

void rtpghi_tilde_window(t_rtpghi_tilde *x, t_symbol s) {
		
	rtpghi_tilde_recreatestate(x, s, x->blocksize, x->overlap);
}

void *rtpghi_tilde_new(t_symbol *s, int argc, t_atom *argv) {
	t_rtpghi_tilde *x = (t_rtpghi_tilde *)pd_new(rtpghi_tilde_class);
	x->window_type_pd = NULL;
	x->overlap = 4.0;
	x->tol_pd = 0.000001;
	x->do_causal_pd = 1.0;
	
	switch (argc) {				//to do: typechecks and value checks)
		case 4: default:
			x->window_type_pd = atom_getsymbol(argv+3);
		case 3:
			x->do_causal_pd = atom_getfloat(argv+2);
		case 2:
			x->tol_pd = atom_getfloat(argv+1);
		case 1:
			x->overlap = atom_getfloat(argv);
			break;
		case 0:
			break;
	}

	post("win_def %p, do_causal %f, tol %f, ol %f", x->window_type_pd, x->do_causal_pd, x->tol_pd, x->overlap);
    x->sta_pd=NULL;
    x->c = NULL;
	x->blocksize = 0;
	
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
  return (void *)x;
}

void rtpghi_tilde_free(t_rtpghi_tilde *x, t_signal **sp) {
	
	if (x->sta_pd != NULL) {
		post("destroyed state at adrr: %p\n", &(x->sta_pd));
		rtpghi_done(&(x->sta_pd));
		freebytes(x->c, (sp[0]->s_n) * sizeof *(x->c));
	}
}

void rtpghi_tilde_setup(void) {
   rtpghi_tilde_class = class_new(gensym("rtpghi~"),
                              (t_newmethod)rtpghi_tilde_new,
                               (t_method)rtpghi_tilde_free,
                              sizeof(t_rtpghi_tilde),
                              CLASS_DEFAULT,
                               A_GIMME,
                              0);

	class_addmethod(rtpghi_tilde_class,
			(t_method)rtpghi_tilde_causal, gensym("causal"),
			A_DEFFLOAT, 0);
	
	class_addmethod(rtpghi_tilde_class,
			(t_method)rtpghi_tilde_tol, gensym("tolerance"),
			A_DEFFLOAT, 0);
	
	class_addmethod(rtpghi_tilde_class,
			(t_method)rtpghi_tilde_overlap, gensym("overlap"),
			A_DEFFLOAT, 0);
	
	class_addmethod(rtpghi_tilde_class,
			(t_method)rtpghi_tilde_window, gensym("window"),
			A_DEFSYMBOL, 0);
	
	class_addmethod(rtpghi_tilde_class,
				   (t_method)rtpghi_tilde_dsp, gensym("dsp"), A_CANT, 0);
	CLASS_MAINSIGNALIN(rtpghi_tilde_class, t_rtpghi_tilde, f);
}

