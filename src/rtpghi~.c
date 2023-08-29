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
	t_float M_pd;
	t_float tol_pd;
	t_float do_causal_pd;
	t_symbol window_type;
	phaseret_rtpghi_state_s* sta_pd;
	ltfat_complex_s *c;

    t_float f;
    t_outlet *out_real;
    t_outlet *out_imag;
   
} t_rtpghi_tilde;

t_int *rtpghi_tilde_perform(t_int *w) {
	
	t_rtpghi_tilde *x = (t_rtpghi_tilde *)(w[1]);
	const float *s =      (const float*)(w[2]);
	int            n =             (int)(w[5]);

	ltfat_complex_s *c = (ltfat_complex_s *) x->c;

	int e = 0;

	if ((s==NULL) || ((x->sta_pd)==NULL)) {
	   post("arrays not initialised");
	}
	else
	{
		e = phaseret_rtpghi_execute_s(x->sta_pd, s, c);
		
		t_sample *out=(t_sample *) (w[3]);
		t_sample *out1= (t_sample *) (w[4]);

		if (e == 0) {
			while (n--) {
				
				if (n >= (x->M_pd)/2-1){
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
			post("error of type: %d", e);
		}
	}

 return (w+6);
}

void rtpghi_tilde_dsp(t_rtpghi_tilde *x, t_signal **sp)
{
    ltfat_int M = x->M_pd;
    
    post("stft_length [M]: %d\n", M);
    const char* win=x->window_type.s_name;
    
    LTFAT_FIRWIN window = ltfat_str2firwin(win);
    double gamma = phaseret_firwin2gamma(window, M);
    post("window normalization parameter [gamma]: %f\n", gamma);
    
    ltfat_int a = M/(x->ol_pd);
    post("hopsize: %d\n", a);
    
    double tol = x->tol_pd;
    post("Tolerance [tol]: %f\n", tol);
    
    int do_causal = 0;
    post("Causal yes=1, no=0, [do_causal]: %d\n", do_causal);
    
    post("%p", (x->sta_pd));
    int init_s = 0;
	ltfat_int w = 1;
	
    init_s = phaseret_rtpghi_init_s(gamma, w, a , M, tol, do_causal, &(x->sta_pd));
    
    //post("%p", (*((x->sta_pd)+4)));
    if (init_s == 0) {
        post("initialised rtpghi plan at adress: %p\n", &(x->sta_pd));
    }
    else {
        post("failed to init, status %d", init_s);
    }
   
   x->c = getbytes(M * sizeof *(x->c));
   post("length of c[]: %d", (M/2+1) * sizeof *(x->c));
   post("s_n: %d", sp[0]->s_n);
   
   dsp_add(rtpghi_tilde_perform, 3,x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void *rtpghi_tilde_new(t_symbol *s, int argc, t_atom *argv)
{

  t_rtpghi_tilde *x = (t_rtpghi_tilde *)pd_new(rtpghi_tilde_class);

    x->ol_pd=atom_getfloat(argv+1);
    x->M_pd=atom_getfloat(argv+2);
    x->tol_pd=atom_getfloat(argv+3);
    x->do_causal_pd=atom_getfloat(argv+4);
    x->window_type= *atom_getsymbol(argv+5);
    x->sta_pd=NULL;
    x-> c = NULL;
    
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    
    x->out_real = getbytes(sizeof (t_sample *));
	x->out_imag = getbytes(sizeof (t_sample *));
	
  return (void *)x;
}


void rtpghi_tilde_free(t_rtpghi_tilde *x)
{
	ltfat_int M = x->M_pd;
	post("destroyed state at adrr: %p\n", &(x->sta_pd));
	phaseret_rtpghi_done_s(&(x->sta_pd));

	freebytes(x->c, (M/2+1) * sizeof *(x->c));
	freebytes(x->out_real, sizeof (t_sample*));
	freebytes(x->out_imag, sizeof (t_sample *));
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
