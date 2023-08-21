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
    t_float W_pd;
    t_float a_pd;
    t_float M_pd;
    t_float tol_pd;
    t_float do_causal_pd;
    t_symbol window_type;
    phaseret_rtpghi_state_s* sta_pd;
    ltfat_complex_s  *c;     ///
    float *s;
    
    t_float f;
    
    //t_inlet *x_in1;
    t_outlet *x_out;
    t_outlet *x_out1;
    
} t_rtpghi_tilde;


t_int *rtpghi_tilde_perform(t_int *w)
{
  //t_sample    *in1 =      (t_sample *)(w[2]);
  float *s =      (float *)(w[1]);
  ltfat_complex_s *c = (ltfat_complex_s *)w[2];
  //t_sample    *in2 =      (t_sample *)(w[2]);
  t_sample    *out =      (t_sample *)(w[3]);
  t_sample    *out1 =      (t_sample *)(w[4]);
  int            n =             (int)(w[5]);
  t_rtpghi_tilde *x = (t_rtpghi_tilde *)(w[6]);
    
    t_sample r = 0;
    t_sample i = 0;
  
    

    phaseret_rtpghi_execute_s(x->sta_pd, s, c);
    
    //t_sample *real = NULL;
    //t_sample *imag = NULL;
    
    
    while (n--) {
        //ltfat_complex_s *c_samp = *c_++;
        r = (t_sample) crealf(*c++);
        i =  (t_sample) cimagf(*c++);
        
        //(x->c) = *(x->c)++;
        //t_sample real = (t_sample) cimag(*(x->c)++);
        //t_sample imag = (t_sample) creal(*(x->c)++);
                    
        *out++ = r;
        *out1++ = i;
    }
  return (w+7);
}

void rtpghi_tilde_dsp(t_rtpghi_tilde *x, t_signal **sp)
{
    
    
    x-> c = ltfat_malloc_sc(sp[0]->s_n);
  dsp_add(rtpghi_tilde_perform, 6, sp[0]->s_vec, x->c , sp[1]->s_vec,
          sp[2]->s_vec, sp[0]->s_n, x);
}



void *rtpghi_tilde_new(t_symbol *s, int argc, t_atom *argv)
{

  t_rtpghi_tilde *x = (t_rtpghi_tilde *)pd_new(rtpghi_tilde_class);

    x->W_pd=atom_getfloat(argv);
    x->a_pd=atom_getfloat(argv+1);
    x->M_pd=atom_getfloat(argv+2);
    x->tol_pd=atom_getfloat(argv+3);
    x->do_causal_pd=atom_getfloat(argv+4);
    x->window_type= *atom_getsymbol(argv+5);
    x->sta_pd=NULL;
    x-> c = NULL;
    x-> s = NULL;
  
    
    ltfat_int M = x->M_pd;
    post("stft_length [M]: %d\n", M);
    const char* win=x->window_type.s_name;
    
    LTFAT_FIRWIN window = ltfat_str2firwin(win);
    double gamma = phaseret_firwin2gamma(window, M);
    post("window normalization parameter [gamma]: %f\n", gamma);
    
    ltfat_int W = x->W_pd;
    post("number of audio channels [W]: %d\n", W);
    
    ltfat_int a = x->a_pd;
    post("a: %d\n", a);
    double tol = x->tol_pd;
    post("Tolerance [tol]: %f\n", tol);
    
    int do_causal = 0;
    post("Causal yes=1, no=0, [do_causal]: %d\n", do_causal);
    
    phaseret_rtpghi_init_s(gamma, W, a , M, tol, do_causal, &(x->sta_pd));
    
    post("initialised rtpghi plan at adress: %p\n", &(x->sta_pd));

    //x->x_in1=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->x_out = outlet_new(&x->x_obj, gensym("signal"));
    x->x_out1 = outlet_new(&x->x_obj, gensym("signal"));
    
  return (void *)x;
}


void rtpghi_tilde_free(t_rtpghi_tilde *x)
{
    post("destroyed state at adrr: %p\n", &(x->sta_pd));
    phaseret_rtpghi_done_s(&(x->sta_pd));
    ltfat_free(x->c);
    //inlet_free(x->x_in1);
    outlet_free(x->x_out);
    outlet_free(x->x_out1);
    
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
