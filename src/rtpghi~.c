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
   t_float ol_pd;
   t_float M_pd;
   t_float tol_pd;
   t_float do_causal_pd;
   t_symbol window_type;
   phaseret_rtpghi_state_s* sta_pd;
   ltfat_complex_s *c;

   t_float f;
   
   t_sample **out;
   
} t_rtpghi_tilde;

t_int *rtpghi_tilde_perform(t_int *w)
{
   t_rtpghi_tilde *x = (t_rtpghi_tilde *)(w[1]);
   const float *s =      (const float*)(w[2]);
   int            n =             (int)(w[3]);
   
   ltfat_complex_s *c = (ltfat_complex_s *) x->c;
   
   int e = 0;
   
   if ((s==NULL) || ((x->sta_pd)==NULL)) {
       post("arrays not initialised");
   }
   else
   {
       e = phaseret_rtpghi_execute_s(x->sta_pd, s, c);
       t_sample *out=(t_sample *) x->out[0];
       t_sample *out1= (t_sample *) x->out[1];
       //post("e: %d", e);
       //post("complex signal: %f + i%f\n",creal(*(c+20)), cimag(*(c+20)));
       while (n--) {
           
           *out = creal(*c);
           *out1 = cimag(*c++);
       }
   }

 return (w+4);
}

void rtpghi_tilde_dsp(t_rtpghi_tilde *x, t_signal **sp)
{
    ltfat_int M = x->M_pd;
    
    post("stft_length [M]: %d\n", M);
    const char* win=x->window_type.s_name;
    
    LTFAT_FIRWIN window = ltfat_str2firwin(win);
    double gamma = phaseret_firwin2gamma(window, M);
    post("window normalization parameter [gamma]: %f\n", gamma);
    
    ltfat_int W = x->W_pd;
    post("number of audio channels [W]: %d\n", W);
    
    
    ltfat_int a = M/(x->ol_pd);
    post("hopsize: %d\n", a);
    
    double tol = x->tol_pd;
    post("Tolerance [tol]: %f\n", tol);
    
    int do_causal = 0;
    post("Causal yes=1, no=0, [do_causal]: %d\n", do_causal);
    
    post("%p", (x->sta_pd));
    int init_s = 0;
    
    init_s = phaseret_rtpghi_init_s(gamma, W, a , M, tol, do_causal, &(x->sta_pd));
    
    //post("%p", (*((x->sta_pd)+4)));
    if (init_s == 0) {
        post("initialised rtpghi plan at adress: %p\n", &(x->sta_pd));
    }
    else {
        post("failed to init, status %d", init_s);
    }
   
   //ltfat_complex_s *c = x->c;
   post("length of c[]: %d", (M) * sizeof *(x->c));
   post("s_n: %d", sp[0]->s_n);
   
   t_sample **dummy=x->out;
   *dummy++=sp[1]->s_vec;
   *dummy=sp[2]->s_vec;
   
   dsp_add(rtpghi_tilde_perform, 3,x, sp[0]->s_vec, sp[0]->s_n);
}

void *rtpghi_tilde_new(t_symbol *s, int argc, t_atom *argv)
{

  t_rtpghi_tilde *x = (t_rtpghi_tilde *)pd_new(rtpghi_tilde_class);

    x->W_pd=atom_getfloat(argv);
    x->ol_pd=atom_getfloat(argv+1);
    x->M_pd=atom_getfloat(argv+2);
    x->tol_pd=atom_getfloat(argv+3);
    x->do_causal_pd=atom_getfloat(argv+4);
    x->window_type= *atom_getsymbol(argv+5);
    x->sta_pd=NULL;
    x-> c = NULL;
    
    
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    
    x->out = (t_sample **) getbytes(2 * sizeof(t_sample *));
    //x->c = (ltfat_complex_s *) getbytes(M * (sizeof *(x->c)) * (x->ol_pd));
    //x->dummy_buffer = (ltfat_complex_s *) ltfat_malloc_sc((M) * 32);
    x->out[0] = 0;
    x->out[1] = 0;
        
  return (void *)x;
}


void rtpghi_tilde_free(t_rtpghi_tilde *x)
{
   ltfat_int M = x->M_pd;
   post("destroyed state at adrr: %p\n", &(x->sta_pd));
   phaseret_rtpghi_done_s(&(x->sta_pd));
   post("x->M_pd: %d", M);
   freebytes(x->c, (M) * sizeof *(x->c));
   freebytes(x->out,2 * sizeof(t_sample *));
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
