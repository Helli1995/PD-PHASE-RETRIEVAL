//
//  test.c
//  
//
//  Created by maximilian helligrath on 28.07.23.
//

#include <m_pd.h>
#include <phaseret.h>
#include <ltfat.h>

static t_class *init_plan_class;

typedef struct _init_plan {
    t_object  x_obj;
    t_float W_pd;
    t_float a_pd;
    t_float M_pd;
    t_float tol_pd;
    t_float do_causal_pd;
    //t_symbol win_type;
    
} t_init_plan;

void init_plan(t_init_plan *x)
{

    ltfat_int M = x->M_pd;
    post("stft_length [M]: %d\n", M);
    
    LTFAT_FIRWIN window = ltfat_str2firwin("hann");
    double gamma = phaseret_firwin2gamma(window, M);
    post("window normalization parameter [gamma]: %f\n", gamma);
    
    ltfat_int W = x->W_pd;
    post("number of audio channels [W]: %d\n", W);
    
    ltfat_int a = x->a_pd;
    post("a: %d\n", a);
    double tol = x->tol_pd;
    post("Tolerance [tol]: %f\n", tol);
    
    int do_causal = x->do_causal_pd;
    post("Causal yes=1, no=0, [do_causal]: %d\n", do_causal);
    
    phaseret_rtpghi_state_d* sta=NULL;
    
    //outlet_symbol(x->x_obj.ob_outlet, sta);
    
    phaseret_rtpghi_init_d(gamma, W, a, M, tol, do_causal, &sta);
    post("initialised rtpghi plan at adress: %p\n", &sta);
    
    phaseret_rtpghi_done_d(sta);
    
}


void *init_plan_new(t_floatarg W, t_floatarg a, t_floatarg M, t_floatarg tol, t_floatarg do_causal)
{

  t_init_plan *x = (t_init_plan *)pd_new(init_plan_class);

    x->W_pd=W;
    x->a_pd=a;
    x->M_pd=M;
    x->tol_pd=tol;
    x->do_causal_pd=do_causal;
    //outlet_new(&x->x_obj, &s_symbol);
  
  return (void *)x;
}


void init_plan_setup(void) {

    init_plan_class = class_new(gensym("init_plan"),
                               (t_newmethod)init_plan_new,
                               0,
                               sizeof(t_init_plan),
                               CLASS_DEFAULT,
                                A_DEFFLOAT,
                                A_DEFFLOAT,
                                A_DEFFLOAT,
                                A_DEFFLOAT,
                                A_DEFFLOAT,
                                
                               0);

  class_addbang(init_plan_class, init_plan);
}
