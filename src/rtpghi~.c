//  Created by maximilian helligrath on 28.07.23.
//

#include <m_pd.h>
#include <phaseret.h>
#include <ltfat.h>

#if !defined(PD_FLOATSIZE)
  /* normally, our floats (t_float, t_sample,...) are 32bit */
# define PD_FLOATSIZE 32
#endif

#if PD_FLOATSIZE == 32
#define phaseret_rtpghi_init phaseret_rtpghi_init_s
#define phaseret_rtpghi_done phaseret_rtpghi_done_s
#define phaseret_rtpghi_set_tol phaseret_rtpghi_set_tol_s
#define phaseret_rtpghi_set_causal phaseret_rtpghi_set_causal_s
#define phaseret_rtpghi_execute phaseret_rtpghi_execute_s

#elif PD_FLOATSIZE == 64
#define phaseret_rtpghi_init phaseret_rtpghi_init_d
#define phaseret_rtpghi_done phaseret_rtpghi_done_d
#define phaseret_rtpghi_set_tol phaseret_rtpghi_set_tol_d
#define phaseret_rtpghi_set_causal phaseret_rtpghi_set_causal_d
#define phaseret_rtpghi_execute phaseret_rtpghi_execute_d
#endif

static t_class *rtpghi_tilde_class;
typedef struct _rtpghi_tilde {
	t_object  x_obj;
	t_float ol_pd;
	t_float tol_pd;
	t_float do_causal_pd;
	t_symbol *window_type_pd;
	phaseret_rtpghi_state_s* sta_pd;
	ltfat_complex_s *c;
	ltfat_int blocksize;
    t_float f;
} t_rtpghi_tilde;

t_int *rtpghi_tilde_perform(t_int *w) {
	
	t_rtpghi_tilde *x = (t_rtpghi_tilde *)(w[1]);
	t_sample *s =      (t_sample *) (w[2]);
	t_sample *out=(t_sample *) (w[3]);
	t_sample *out1= (t_sample *) (w[4]);
	int            n =             (int)(w[5]);

	ltfat_complex_s *c = (ltfat_complex_s *) x->c;
	int e = 0;

	if ((s==NULL) || ((x->sta_pd)==NULL)) {
	   pd_error(x, "arrays not initialised");
	}
	
	else
	{
		e = phaseret_rtpghi_execute(x->sta_pd, s, c);

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

void rtpghi_tilde_dsp(t_rtpghi_tilde *x, t_signal **sp)
{
	//post("start DSP");
    ltfat_int M = (ltfat_int) sp[0]->s_n;
	if ((x->blocksize) == 0) {
		(x->blocksize) = M;
	}
    
    //post("stft_length [M]: %d\n", M);
	const char win_[4] = "hann";
	LTFAT_FIRWIN window;
	
	if (x->window_type_pd != NULL) {
		window = ltfat_str2firwin(x->window_type_pd->s_name);
	}
	else {
		window = ltfat_str2firwin(win_);
	}
	
    double gamma = phaseret_firwin2gamma(window, M);
    //post("window normalization parameter [gamma]: %f\n", gamma);
    
    ltfat_int a = M/(x->ol_pd);
    //post("hopsize: %d\n", a);
    
    double tol = x->tol_pd;
    //post("Tolerance [tol]: %f\n", tol);
    
    int do_causal = x->do_causal_pd;
    //post("Causal yes=1, no=0, [do_causal]: %d\n", do_causal);
	
	
	// Check for existing ltfat_state (to do: implement init only when input param. changes)
	int init_s;
	ltfat_int w = 1;
	
	if (M != (x->blocksize)) {
		post("destroyed state at adrr: %p\n", &(x->sta_pd));
		phaseret_rtpghi_done(&(x->sta_pd));
		x->sta_pd=NULL;
		x->blocksize = M;
	}
	
	if  (x->sta_pd == NULL) {
		init_s = phaseret_rtpghi_init(w, a , M, gamma, tol, do_causal, &(x->sta_pd));
		
		if (init_s == 0) {
			post("initialised rtpghi plan at adress: %p\n", &(x->sta_pd));
		}
		else {
			pd_error(x, "failed to init, status %d", init_s);
		}
	}
	
	if (x->c != NULL) {
		freebytes(x->c, M * sizeof *(x->c));
		x->c = NULL;
	}
	if (x->c == NULL) {
		x->c = getbytes(M * sizeof *(x->c));
	}
	
   //post("length of c[] [bit]: %d", M * sizeof *(x->c));
   //post("s_n: %d", sp[0]->s_n);
   
   dsp_add(rtpghi_tilde_perform, 5,x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void rtpghi_tilde_causal(t_rtpghi_tilde *x, t_floatarg f)
{
	if ((f == 0.0) || (f == 1.0)) {
		int causal = f;
		phaseret_rtpghi_set_causal(x->sta_pd, causal);
	}
	else {
		pd_error(x, "failed to set (a)causal, input has to be either 0 or 1, but got: %f", f);
	}
}
void rtpghi_tilde_tol(t_rtpghi_tilde *x, t_floatarg f)
{
	if ((f > 0.0) && (f < 1.0)) {
		double tol = f;
		phaseret_rtpghi_set_tol(x->sta_pd, tol);
	}
	else {
		pd_error(x, "failed to set tol, input float has to be  > 0 && <1, but got: %f", f);
	}
}

void rtpghi_tilde_overlap(t_rtpghi_tilde *x, t_floatarg f)
{
	if ((f > 0.0) && (f <= x->blocksize)) {
		phaseret_rtpghi_done(&(x->sta_pd));
		post("destroyed state at adrr: %p\n", &(x->sta_pd));
		x->sta_pd=NULL;
		x->ol_pd = f;
		ltfat_int M = x->blocksize;
		if ((x->blocksize) == 0) {
			(x->blocksize) = M;
		}
		const char win_[4] = "hann";
		LTFAT_FIRWIN window;
		if (x->window_type_pd != NULL) {
			window = ltfat_str2firwin(x->window_type_pd->s_name);
		}
		else {
			window = ltfat_str2firwin(win_);
		}
		double gamma = phaseret_firwin2gamma(window, M);
		ltfat_int a = M/(x->ol_pd);
		double tol = x->tol_pd;
		int do_causal = x->do_causal_pd;
		int init_s;
		ltfat_int w = 1;
		init_s = phaseret_rtpghi_init(w, a , M, gamma, tol, do_causal, &(x->sta_pd));
		if (init_s == 0) {
			post("initialised rtpghi plan at adress: %p\n", &(x->sta_pd));
		}
		else {
			pd_error(x, "failed to init, status %d", init_s);
		}
	}
	else {
		pd_error(x, "failed to set overlap, input float has to be  > 0 && <= blocksize, but got: %f", f);
	}
}

void rtpghi_tilde_window(t_rtpghi_tilde *x, t_symbol s)
{
		
		LTFAT_FIRWIN window;
		window = ltfat_str2firwin(s.s_name);
	LTFAT_FIRWIN check = -6;
	if (window != check) {
		
		phaseret_rtpghi_done(&(x->sta_pd));
		post("destroyed state at adrr: %p\n", &(x->sta_pd));
		x->sta_pd=NULL;
		ltfat_int M = x->blocksize;
		if ((x->blocksize) == 0) {
			(x->blocksize) = M;
		}
		double gamma = phaseret_firwin2gamma(window, M);
		ltfat_int a = M/(x->ol_pd);
		double tol = x->tol_pd;
		int do_causal = x->do_causal_pd;
		int init_s;
		ltfat_int w = 1;
		init_s = phaseret_rtpghi_init(w, a , M, gamma, tol, do_causal, &(x->sta_pd));
		if (init_s == 0) {
			post("initialised rtpghi plan at adress: %p\n", &(x->sta_pd));
		}
		else {
			pd_error(x, "failed to init, status %d", init_s);
		}
	}
		else {
			pd_error(x, "failed to set new window, type not supported: %s", s.s_name);
		}
}

/*void rtpghi_tilde_res(t_rtpghi_tilde *x)
{
	phaseret_rtpghi_reset_s(x->sta_pd);
}*/

void *rtpghi_tilde_new(t_symbol *s, int argc, t_atom *argv)
{

  t_rtpghi_tilde *x = (t_rtpghi_tilde *)pd_new(rtpghi_tilde_class);
	x->window_type_pd = NULL;
	x->ol_pd = 4.0;
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
			x->ol_pd = atom_getfloat(argv);
			break;
		case 0:
			break;
	}

	post("win_def %p, do_causal %f, tol %f, ol %f", x->window_type_pd, x->do_causal_pd, x->tol_pd, x->ol_pd);
    x->sta_pd=NULL;
    x->c = NULL;
	x->blocksize = 0;
    
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
  return (void *)x;
}

void rtpghi_tilde_free(t_rtpghi_tilde *x, t_signal **sp)
{
	if (x->sta_pd != NULL){
		
		post("destroyed state at adrr: %p\n", &(x->sta_pd));
		phaseret_rtpghi_done(&(x->sta_pd));
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
	//to do: maybe change tol_pd on runtime (while DSP turned ON)
	/*class_addbang  (rtpghi_tilde_class, rtpghi_tilde_res);*/
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
	class_sethelpsymbol(rtpghi_tilde_class, gensym("rtpghi~"));
	}

