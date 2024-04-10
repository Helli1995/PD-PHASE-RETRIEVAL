//  Created by maximilian helligrath on 28.03.24.
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

#define rtisi_reset PHASERET_NAME(rtisila_reset)
#define rtisi_done PHASERET_NAME(rtisila_done)
#define rtisi_set_lookahead PHASERET_NAME(rtisila_set_lookahead)
#define rtisi_init PHASERET_NAME(rtisila_init_win)
#define rtisi_execute PHASERET_NAME(rtisila_execute)
#define ltfat_complex LTFAT_COMPLEX
#define rtisi_state PHASERET_NAME(rtisila_state)


static t_class *rtisi_tilde_class;
typedef struct _rtisi_tilde {
	t_object  x_obj;
	t_float overlap;
	t_float look_ahead_pd;
	t_float max_iter_pd;
	t_symbol *window_type_pd;
	rtisi_state* sta_pd;
	ltfat_complex *c;
	ltfat_int blocksize;
	LTFAT_FIRWIN window;
	int audio_size;
    t_float f;
} t_rtisi_tilde;

t_int *rtisi_tilde_perform(t_int *w) {
	
	t_rtisi_tilde *x = (t_rtisi_tilde *)(w[1]);
	t_sample *s =      (t_sample *) (w[2]);
	t_sample *out=(t_sample *) (w[3]);
	t_sample *out1= (t_sample *) (w[4]);
	int            n =             (int)(w[5]);

	ltfat_complex *c = (ltfat_complex *) x->c;
	int e = 0;
	if (x->sta_pd == NULL || s==NULL) {
		*out++ = 0.0;
		*out1++ = 0.0;
	}
	else {
		e = rtisi_execute(x->sta_pd, s, c);
	}

	if (e == 0) {
		while (n--) {
			*out++ = creal(*c);
			*out1++ = cimag(*c++);
		}
	}
	else {
		pd_error(x, "error of type: %d", e);
	}
	
 return (w+6);
	
}


void rtisi_tilde_recreatestate(t_rtisi_tilde *x, LTFAT_FIRWIN win, ltfat_int blocksize, int overlap, int max_iter, int win_check) {
	if (win_check == 1) {
		if(x->sta_pd && (win != x->window || blocksize != x->blocksize || overlap != x->overlap || max_iter != x->max_iter_pd)) {
			rtisi_done(&(x->sta_pd));
			x->sta_pd=NULL;
			post("sta zeroed");
		}
		x->window = win;
	}
	else {
		if(x->sta_pd && (blocksize != x->blocksize || overlap != x->overlap || max_iter != x->max_iter_pd)) {
			rtisi_done(&(x->sta_pd));
			x->sta_pd=NULL;
			post("sta zeroed");
		}
	}
	if (!x->sta_pd) {
		ltfat_int w = 1;
		x->blocksize = blocksize;

		x->overlap = overlap;
		x->max_iter_pd = max_iter;
		double look_ahead = x->look_ahead_pd;
		
		if (rtisi_init(x->window, x->blocksize, w, blocksize/(x->overlap), x->blocksize, look_ahead, max_iter, &(x->sta_pd))) {
			pd_error(x, "failed to init state, DSP turned off?");
			x->sta_pd=NULL;
			}
		}
}

void rtisi_tilde_dsp(t_rtisi_tilde *x, t_signal **sp) {

	ltfat_int M = (ltfat_int) sp[0]->s_n;
	x->audio_size = sp[0]->s_n;
	
	if ((x->blocksize) == 0) {
		(x->blocksize) = M;
	}

	if (x->window_type_pd != NULL) {
		x->window = ltfat_str2firwin(x->window_type_pd->s_name);
	}

	rtisi_tilde_recreatestate(x, x->window, M, x->overlap,x->max_iter_pd, 0);
	
	if (x->c != NULL) {
		freebytes(x->c, M * sizeof *(x->c));
		x->c = NULL;
	}
	if (x->c == NULL) {
		x->c = getbytes(M * sizeof *(x->c));
	}
	
	if (x->sta_pd == NULL) {
	   pd_error(x, "no output processing due to initalisation failure");
	}
	dsp_add(rtisi_tilde_perform, 5,x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void rtisi_tilde_look_ahead(t_rtisi_tilde *x, t_floatarg f) {
	
	if ((f > 0.0) && (f <= 32.0)) {
		int look_ahead = f;
		rtisi_set_lookahead(x->sta_pd, look_ahead);
	}
	else {
		pd_error(x, "failed to set lookahead, input has to be in range of 1-32, but got: %f", f);
	}
}

void rtisi_tilde_overlap(t_rtisi_tilde *x, t_floatarg f) {
	if ((f > 0.0) && (f <= x->blocksize)) {
		rtisi_tilde_recreatestate(x, x->window, x->blocksize, f,x->max_iter_pd, 0);
	}
	else {
		pd_error(x, "failed to set overlap, input float has to be  > 0 && <= blocksize, but got: %f", f);
	}
}

void rtisi_tilde_max_iter(t_rtisi_tilde *x, t_floatarg f) {
	if ((f > 0.0) && (f <= x->look_ahead_pd)) {
		rtisi_tilde_recreatestate(x, x->window, x->blocksize, x->overlap, f, 0);
	}
	else {
		pd_error(x, "failed to set overlap, input float has to be  > 0 && <= blocksize, but got: %f", f);
	}
}

void rtisi_tilde_window(t_rtisi_tilde *x, t_symbol* s) {
	
	LTFAT_FIRWIN new_win = ltfat_str2firwin(s->s_name);
	
	LTFAT_FIRWIN check = -6;
	if (new_win == check) {
		pd_error(x, "failed to set new window type, has to match member of list in the helpfile");
	}
		
	rtisi_tilde_recreatestate(x, new_win, x->blocksize, x->overlap, x->max_iter_pd, 1);
}

void *rtisi_tilde_new(t_symbol *s, int argc, t_atom *argv) {
	t_rtisi_tilde *x = (t_rtisi_tilde *)pd_new(rtisi_tilde_class);
	x->window_type_pd = NULL;
	x->overlap = 4.0;
	x->max_iter_pd = 32.0;
	x->look_ahead_pd = 16.0;
	
	switch (argc) {				//to do: typechecks and value checks)
		case 4: default:
			x->window_type_pd = atom_getsymbol(argv+3);
		case 3:
			x->look_ahead_pd = atom_getfloat(argv+2);
		case 2:
			x->max_iter_pd = atom_getfloat(argv+1);
		case 1:
			x->overlap = atom_getfloat(argv);
			break;
		case 0:
			break;
	}

	post("current_window: %p, look_ahead: %f, maximum_iterations: %f, overlap_factor: %f", x->window_type_pd, x->look_ahead_pd, x->max_iter_pd, x->overlap);
    x->sta_pd = NULL;
    x->c = NULL;
	x->blocksize = 0;
	
	if (x->window_type_pd == NULL) {
		//t_symbol window = "hanning";
		x->window = LTFAT_HANN;
	}
	
    outlet_new(&x->x_obj, &s_signal);
	outlet_new(&x->x_obj, &s_signal);
	return (void *)x;
}

void rtisi_tilde_free(t_rtisi_tilde *x) {
	
	if (x->sta_pd != NULL) {
		post("destroyed state at adrr: %p\n", &(x->sta_pd));
		rtisi_done(&(x->sta_pd));
		x->sta_pd = NULL;
		freebytes(x->c, (x->audio_size) * sizeof *(x->c));
	}
}

void rtisi_tilde_setup(void) {
	rtisi_tilde_class = class_new(gensym("rtisi~"),
                              (t_newmethod)rtisi_tilde_new,
                               (t_method)rtisi_tilde_free,
                              sizeof(t_rtisi_tilde),
                              CLASS_DEFAULT,
                               A_GIMME,
                              0);

	class_addmethod(rtisi_tilde_class,
			(t_method)rtisi_tilde_look_ahead, gensym("look_ahead"),
			A_DEFFLOAT, 0);
	
	class_addmethod(rtisi_tilde_class,
			(t_method)rtisi_tilde_overlap, gensym("overlap"),
			A_DEFFLOAT, 0);
	class_addmethod(rtisi_tilde_class,
			(t_method)rtisi_tilde_max_iter, gensym("max_iter"),
			A_DEFFLOAT, 0);
	class_addmethod(rtisi_tilde_class,
			(t_method)rtisi_tilde_window, gensym("window"),
			A_DEFSYMBOL, 0);
	class_addmethod(rtisi_tilde_class,
				   (t_method)rtisi_tilde_dsp, gensym("dsp"), A_CANT, 0);
	CLASS_MAINSIGNALIN(rtisi_tilde_class, t_rtisi_tilde, f);
}
