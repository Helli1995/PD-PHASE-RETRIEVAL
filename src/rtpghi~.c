//
//  test.c
//
//
//  Created by maximilian helligrath on 28.07.23.
//

#include <m_pd.h>
#include <phaseret.h>
#include <ltfat.h>

static t_class *rtpghi_tilde_class;
typedef struct _rtpghi_tilde {
	t_object  x_obj;
	t_float ol_pd;
	t_float tol_pd;
	t_float do_causal_pd;
	t_symbol *window_type_pd;
	phaseret_rtpghi_state_s* sta_pd;
	ltfat_complex_s *c;
	t_sample *s2;
    t_float f;
} t_rtpghi_tilde;

t_int *rtpghi_tilde_perform(t_int *w) {
	
	t_rtpghi_tilde *x = (t_rtpghi_tilde *)(w[1]);
	t_sample *s =      (t_sample *) (w[2]);
	t_sample *out=(t_sample *) (w[3]);
	t_sample *out1= (t_sample *) (w[4]);
	int            n =             (int)(w[5])4;
	//int            n =             ((int)(w[5]))/8;

	ltfat_complex_s *c = (ltfat_complex_s *) x->c;
	t_sample *s2 = (t_sample *) x->s2;
	
	int n2 = n/2;
	while (n2--) {
		*s2++ = *s++;
	}

	int e = 0;

	if ((s==NULL) || ((x->sta_pd)==NULL)) {
	   pd_error(x, "arrays not initialised");
	}
	
	else
	{
		e = phaseret_rtpghi_execute_s(x->sta_pd, s2, c);

		if (e == 0) {
			while (n--) {
				
				if (n >= n/2-1){
					*out++ = creal(*c);
					*out1++ = cimag(*c++);
				}
				else {
					
					*out++ = 0.f;
					*out1++ = 0.f;
				}
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
	post("start DSP");
    ltfat_int M = (ltfat_int) sp[0]->s_n;
	//ltfat_int M = (ltfat_int) sp[0]->s_n/4;
    
    post("stft_length [M]: %d\n", M);
	const char win_[4] = "hann";
	LTFAT_FIRWIN window;
	
	if (x->window_type_pd != NULL) {
		window = ltfat_str2firwin(x->window_type_pd->s_name);
	}
	else {
		window = ltfat_str2firwin(win_);
	}
	
    double gamma = phaseret_firwin2gamma(window, M);
    post("window normalization parameter [gamma]: %f\n", gamma);
    
    ltfat_int a = M/(x->ol_pd);
    post("hopsize: %d\n", a);
    
    double tol = x->tol_pd;
    post("Tolerance [tol]: %f\n", tol);
    
    int do_causal = x->do_causal_pd;
    post("Causal yes=1, no=0, [do_causal]: %d\n", do_causal);
	
	
	// Check for existing ltfat_state (to do: implement init only when input param. changes)
	int init_s;
	ltfat_int w = 1;
	
	if (x->sta_pd != NULL) {
		post("destroyed state at adrr: %p\n", &(x->sta_pd));
		phaseret_rtpghi_done_s(&(x->sta_pd));
		x->sta_pd=NULL;
	}
	if  (x->sta_pd == NULL) {
		init_s = phaseret_rtpghi_init_s(gamma, w, a , M, tol, do_causal, &(x->sta_pd));
		
		if (init_s == 0) {
			post("initialised rtpghi plan at adress: %p\n", &(x->sta_pd));
		}
		else {
			pd_error(x, "failed to init, status %d", init_s);
		}
	}
	
	if (x->c != NULL) {
		freebytes(x->c, M * sizeof *(x->c));
		freebytes(x->s2, (M/2+1) * sizeof *(x->c));
		x->c = NULL;
		x->s2 = NULL;
	}
	if (x->c == NULL) {
		x->c = getbytes(M * sizeof *(x->c));
		x->s2 = getbytes((M/2+1) * sizeof *(x->c));
	}
	
   post("length of c[] [bit]: %d", M * sizeof *(x->c));
   post("s_n: %d", sp[0]->s_n);
	//post("overlap_intern:", sp[0]->s_overlap);
   
   dsp_add(rtpghi_tilde_perform, 5,x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

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
    
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
  return (void *)x;
}

void rtpghi_tilde_free(t_rtpghi_tilde *x, t_signal **sp)
{
	if (x->sta_pd != NULL){
		
		post("destroyed state at adrr: %p\n", &(x->sta_pd));
		phaseret_rtpghi_done_s(&(x->sta_pd));
		freebytes(x->c, (sp[0]->s_n) * sizeof *(x->c));
		//freebytes(x->c, (sp[0]->s_n)/8 * sizeof *(x->c));
		freebytes(x->s2, ((sp[0]->s_n)/2+1) * sizeof *(x->c));
		//freebytes(x->s2, (((sp[0]->s_n)/8)/2+1) * sizeof *(x->c));
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
	//to do: add methods to change do_causal_pd, tol_pd on runtime
   class_addmethod(rtpghi_tilde_class,
                   (t_method)rtpghi_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(rtpghi_tilde_class, t_rtpghi_tilde, f);
}
