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

    t_float f;
   
} t_rtpghi_tilde;

t_int *rtpghi_tilde_perform(t_int *w) {
	
	t_rtpghi_tilde *x = (t_rtpghi_tilde *)(w[1]);
	t_sample *s =      (t_sample *) (w[2]);
	int            n =             (int)(w[5]);

	ltfat_complex_s *c = (ltfat_complex_s *) x->c;

	int e = 0;

	if ((s==NULL) || ((x->sta_pd)==NULL)) {
	   pd_error(x, "arrays not initialised");
	}
	else
	{
		e = phaseret_rtpghi_execute_s(x->sta_pd, s, c);
		
		t_sample *out=(t_sample *) (w[3]);
		t_sample *out1= (t_sample *) (w[4]);

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
    
    post("stft_length [M]: %d\n", M);
    const char* win=x->window_type_pd->s_name;
    
    LTFAT_FIRWIN window = ltfat_str2firwin(win);
    double gamma = phaseret_firwin2gamma(window, M);
    post("window normalization parameter [gamma]: %f\n", gamma);
    
    ltfat_int a = M/(x->ol_pd);
    post("hopsize: %d\n", a);
    
    double tol = x->tol_pd;
    post("Tolerance [tol]: %f\n", tol);
    
    int do_causal = x->do_causal_pd;
    post("Causal yes=1, no=0, [do_causal]: %d\n", do_causal);
	
	
	// Check for existing ltfat_state (to do: implement init only when input param.)
	int init_s;
	ltfat_int w = 1;
	
	if (x->sta_pd == NULL) {
		init_s = phaseret_rtpghi_init_s(gamma, w, a , M, tol, do_causal, &(x->sta_pd));
		post("failed to init, status %d", init_s);
		if (init_s == 0) {
			post("initialised rtpghi plan at adress: %p\n", &(x->sta_pd));
		}
		else {
			post("failed to init, status %d", init_s);
			pd_error(x, "failed to init, status %d", init_s);
		}
	}
	else {
		
		post("destroyed state at adrr: %p\n", &(x->sta_pd));
		phaseret_rtpghi_done_s(&(x->sta_pd));
		init_s = phaseret_rtpghi_init_s(gamma, w, a , M, tol, do_causal, &(x->sta_pd));
		if (init_s == 0) {
			post("initialised rtpghi plan at adress: %p\n", &(x->sta_pd));
		}
		else {
			pd_error(x, "failed to init, status %d", init_s);
		}
	}
	
   
   x->c = getbytes(M * sizeof *(x->c));
   post("length of c[]: %d", M * sizeof *(x->c));
   post("s_n: %d", sp[0]->s_n);
   
   dsp_add(rtpghi_tilde_perform, 3,x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void *rtpghi_tilde_new(t_symbol *s, int argc, t_atom *argv)
{

  t_rtpghi_tilde *x = (t_rtpghi_tilde *)pd_new(rtpghi_tilde_class);
	
	switch (argc) {				//to do: typechecks and value checks)
		case 4: default:
			x->ol_pd=atom_getfloat(argv);
			x->tol_pd=atom_getfloat(argv+1);
			x->do_causal_pd=atom_getfloat(argv+2);
			x->window_type_pd= atom_getsymbol(argv+3);
			post("4");
			break;
		case 3:
			x->ol_pd=atom_getfloat(argv);
			x->tol_pd=atom_getfloat(argv+1);
			x->do_causal_pd=atom_getfloat(argv+2);
			x->window_type_pd->s_name= "hann";
			post("3");
			break;
		case 2:
			x->ol_pd=atom_getfloat(argv);
			x->tol_pd=atom_getfloat(argv+1);
			x->do_causal_pd=1;
			x->window_type_pd->s_name= "hann";
			post("2");
			break;
		case 1:
			x->ol_pd=atom_getfloat(argv);
			x->tol_pd=0.000001;
			x->do_causal_pd=1.f;
			x->window_type_pd->s_name= "hann";
			post("1");
			break;
		case 0:
			x->ol_pd=1.f;
			x->tol_pd=0.000001;
			x->do_causal_pd=1;
			x->window_type_pd->s_name= "hann";
			post("0");
			break;
	}
	
	post("dummy");
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
                   (t_method)rtpghi_tilde_dsp, gensym("dsp"), A_CANT, 0);
   CLASS_MAINSIGNALIN(rtpghi_tilde_class, t_rtpghi_tilde, f);
}

