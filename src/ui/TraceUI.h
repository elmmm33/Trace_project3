//
// rayUI.h
//
// The header file for the UI part
//

#ifndef __rayUI_h__
#define __rayUI_h__

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>

#include <FL/fl_file_chooser.H>		// FLTK file chooser

#include "TraceGLWindow.h"

class TraceUI {
public:
	TraceUI();

	// The FLTK widgets
	Fl_Window*			m_mainWindow;
	Fl_Menu_Bar*		m_menubar;

	Fl_Slider*			m_sizeSlider;
	Fl_Slider*			m_depthSlider;
	Fl_Light_Button*	m_shadowSwitch;
	Fl_Light_Button*	m_softShadowSwitch;
	Fl_Light_Button*	m_reflectionSwitch;
	Fl_Slider*			m_glossyReflectionSlider;

	Fl_Light_Button*	m_fresnelSwitch;

	Fl_Slider*			m_fresnelSlider;

	Fl_Light_Button*	m_refractionSwitch;
	Fl_Slider*			m_threadSlider;
	Fl_Slider*			m_intensityThresholdSlider;
	Fl_Slider*			m_superSamplingSlider;

	Fl_Light_Button*	m_distanceSwitch;
	Fl_Slider*			m_distanceConstantSlider;
	Fl_Slider*			m_distanceLinearSlider;
	Fl_Slider*			m_distanceQuadraticSlider;

	Fl_Button*			m_renderButton;
	Fl_Button*			m_stopButton;

	TraceGLWindow*		m_traceGlWindow;

	// member functions
	void show();

	void setRayTracer(RayTracer *tracer);

	int	getSize() const
	{
		return m_nSize;
	}

	int	getDepth() const
	{
		return m_nDepth;
	}

	bool IsEnableShadow() const
	{
		return m_is_enable_shadow;
	}

	bool IsEnableSoftShadow() const
	{
		return m_is_enable_soft_shadow;
	}

	bool IsEnableReflection() const
	{
		return m_is_enable_reflection;
	}

	double GetGlossyReflectionSample() const
	{
		return m_glossy_reflection_sample;
	}

	bool IsEnableFresnel() const
	{
		return m_is_enable_fresnel;
	}

	double GetFresnelRatio() const
	{
		return m_fresnel_ratio;
	}

	bool IsEnableRefraction() const
	{
		return m_is_enable_refraction;
	}

	int	GetThread() const
	{
		return m_thread;
	}

	double GetIntensityThreshold() const
	{
		return m_intensity_threshold;
	}

	int	GetSuperSampling() const
	{
		return m_super_sampling;
	}

	bool IsOverideDistance() const
	{
		return m_is_overide_distance;
	}

	double GetDistanceConstant() const
	{
		return m_distance_constant;
	}

	double GetDistanceLinear() const
	{
		return m_distance_linear;
	}

	double GetDistanceQuadratic() const
	{
		return m_distance_quadratic;
	}

private:
	RayTracer*	raytracer;

	int	m_nSize;
	int	m_nDepth;
	bool m_is_enable_shadow;
	bool m_is_enable_soft_shadow;
	bool m_is_enable_reflection;
	int m_glossy_reflection_sample;
	bool m_is_enable_fresnel;
	double m_fresnel_ratio;
	bool m_is_enable_refraction;
	int m_thread;
	double m_intensity_threshold;
	int m_super_sampling;
	bool m_is_overide_distance;
	double m_distance_constant;
	double m_distance_linear;
	double m_distance_quadratic;

	const char *m_curr_file;

// static class members
	static Fl_Menu_Item menuitems[];

	static TraceUI* whoami(Fl_Menu_* o);

	static void cb_load_scene(Fl_Menu_* o, void* v);
	static void cb_save_image(Fl_Menu_* o, void* v);
	static void cb_exit(Fl_Menu_* o, void* v);
	static void cb_about(Fl_Menu_* o, void* v);

	static void cb_exit2(Fl_Widget* o, void* v);

	static void cb_sizeSlides(Fl_Widget* o, void* v);
	static void cb_depthSlides(Fl_Widget* o, void* v);
	static void cb_shadowSwitch(Fl_Widget* o, void* v);
	static void cb_softShadowSwitch(Fl_Widget* o, void* v);
	static void cb_reflectionSwitch(Fl_Widget* o, void* v);
	static void cb_glossyReflectionSlides(Fl_Widget* o, void* v);
	static void cb_fresnelSwitch(Fl_Widget* o, void* v);
	static void cb_fresnelSlides(Fl_Widget* o, void* v);
	static void cb_refractionSwitch(Fl_Widget* o, void* v);
	static void cb_threadSlides(Fl_Widget* o, void* v);
	static void cb_intensityThresholdSlides(Fl_Widget* o, void* v);
	static void cb_superSamplingSlides(Fl_Widget* o, void* v);
	static void cb_distanceSwitch(Fl_Widget* o, void* v);
	static void cb_distanceConstantSlides(Fl_Widget* o, void* v);
	static void cb_distanceLinearSlides(Fl_Widget* o, void* v);
	static void cb_distanceQuadraticSlides(Fl_Widget* o, void* v);

	static void cb_render(Fl_Widget* o, void* v);
	static void cb_stop(Fl_Widget* o, void* v);

	void load_scene(const char *file);

	static void RenderWorker(TraceUI *ui, const int from_y, const int to_y,
			const int w);
};

#endif
