//
// TraceUI.h
//
// Handles FLTK integration and other user interface tasks
//
#include <cstdio>
#include <ctime>
#include <cstring>
#include <chrono>
#include <future>
#include <vector>

#include <FL/fl_ask.H>

#include "TraceUI.h"
#include "../RayTracer.h"

static bool done;

//------------------------------------- Help Functions --------------------------------------------
TraceUI* TraceUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ( (TraceUI*)(o->parent()->user_data()) );
}

//--------------------------------- Callback Functions --------------------------------------------
void TraceUI::cb_load_scene(Fl_Menu_* o, void* v)
{
	TraceUI* pUI=whoami(o);

	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		pUI->load_scene(newfile);
	}
}

void TraceUI::load_scene(const char *file)
{
	char buf[256];

	if (raytracer->loadScene(file)) {
		sprintf(buf, "Ray <%s>", file);
		done=true;	// terminate the previous rendering
	} else{
		sprintf(buf, "Ray <Not Loaded>");
	}

	m_mainWindow->label(buf);
	m_curr_file = file;
}

void TraceUI::cb_save_image(Fl_Menu_* o, void* v)
{
	TraceUI* pUI=whoami(o);

	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL) {
		pUI->m_traceGlWindow->saveImage(savefile);
	}
}

void TraceUI::cb_exit(Fl_Menu_* o, void* v)
{
	TraceUI* pUI=whoami(o);

	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}

void TraceUI::cb_exit2(Fl_Widget* o, void* v)
{
	TraceUI* pUI=(TraceUI *)(o->user_data());

	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}

void TraceUI::cb_about(Fl_Menu_* o, void* v)
{
	fl_message("RayTracer Project, FLTK version for CS 341 Spring 2002. Latest modifications by Jeff Maurer, jmaurer@cs.washington.edu");
}

void TraceUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
	TraceUI* pUI=(TraceUI*)(o->user_data());

	pUI->m_nSize=int( ((Fl_Slider *)o)->value() ) ;
	int	height = (int)(pUI->m_nSize / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow( pUI->m_nSize, height );
}

void TraceUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void TraceUI::cb_shadowSwitch(Fl_Widget *o, void*)
{
	((TraceUI*)(o->user_data()))->m_is_enable_shadow ^= true;
}

void TraceUI::cb_softShadowSwitch(Fl_Widget *o, void*)
{
	((TraceUI*)(o->user_data()))->m_is_enable_soft_shadow ^= true;
}

void TraceUI::cb_reflectionSwitch(Fl_Widget *o, void*)
{
	((TraceUI*)(o->user_data()))->m_is_enable_reflection ^= true;
}

void TraceUI::cb_glossyReflectionSlides(Fl_Widget* o, void*)
{
	((TraceUI*)(o->user_data()))->m_glossy_reflection_sample =
			((Fl_Slider*)o)->value();
}

void TraceUI::cb_fresnelSwitch(Fl_Widget *o, void*)
{
	((TraceUI*)(o->user_data()))->m_is_enable_fresnel ^= true;
}

void TraceUI::cb_fresnelSlides(Fl_Widget* o, void*)
{
	((TraceUI*)(o->user_data()))->m_fresnel_ratio =
			((Fl_Slider*)o)->value() / 100.0;
}

void TraceUI::cb_refractionSwitch(Fl_Widget *o, void*)
{
	((TraceUI*)(o->user_data()))->m_is_enable_refraction ^= true;
}

void TraceUI::cb_threadSlides(Fl_Widget* o, void*)
{
	((TraceUI*)(o->user_data()))->m_thread = ((Fl_Slider*)o)->value();
}

void TraceUI::cb_intensityThresholdSlides(Fl_Widget* o, void*)
{
	((TraceUI*)(o->user_data()))->m_intensity_threshold = ((Fl_Slider*)o)->value();
}

void TraceUI::cb_superSamplingSlides(Fl_Widget* o, void*)
{
	((TraceUI*)(o->user_data()))->m_super_sampling = ((Fl_Slider*)o)->value();
}

void TraceUI::cb_distanceSwitch(Fl_Widget *o, void*)
{
	((TraceUI*)(o->user_data()))->m_is_overide_distance ^= true;
}

void TraceUI::cb_distanceConstantSlides(Fl_Widget* o, void*)
{
	((TraceUI*)(o->user_data()))->m_distance_constant = ((Fl_Slider*)o)->value();
}

void TraceUI::cb_distanceLinearSlides(Fl_Widget* o, void*)
{
	((TraceUI*)(o->user_data()))->m_distance_linear = ((Fl_Slider*)o)->value();
}

void TraceUI::cb_distanceQuadraticSlides(Fl_Widget* o, void*)
{
	((TraceUI*)(o->user_data()))->m_distance_quadratic = ((Fl_Slider*)o)->value();
}

void TraceUI::RenderWorker(TraceUI *ui, const int from_y, const int to_y,
		const int w)
{
	for (int y = from_y; y < to_y && !done; ++y)
	{
		for (int x = 0; x < w && !done; ++x)
		{
			ui->raytracer->tracePixel( x, y );
		}
	}
}

void TraceUI::cb_render(Fl_Widget* o, void* v)
{
	char buffer[256];

	TraceUI* pUI=((TraceUI*)(o->user_data()));

	if (pUI->raytracer->sceneLoaded()) {
#ifdef DEBUG
		// reload scene if debug build, to make it easier to debug
		pUI->load_scene(pUI->m_curr_file);
#endif

		int width=pUI->getSize();
		int	height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		pUI->m_traceGlWindow->resizeWindow( width, height );

		pUI->m_traceGlWindow->show();

		pUI->raytracer->traceSetup(width, height);

		// Save the window label
		const char *old_label = pUI->m_traceGlWindow->label();

		// start to render here
		done=false;
		clock_t prev, now;
		prev=clock();

		pUI->m_traceGlWindow->refresh();
		Fl::check();
		Fl::flush();

		vector<future<void>> workers;
		const int partition = height / pUI->GetThread();
		for (int i = 0; i < pUI->GetThread() - 1; ++i)
		{
			workers.push_back(async(launch::async, RenderWorker, pUI,
					partition * i, partition * (i + 1), width));
		}
		workers.push_back(async(launch::async, RenderWorker, pUI,
				partition * (pUI->GetThread() - 1), height, width));

		bool is_all_joined = false;
		do
		{
			// current time
			now = clock();

			// check event every 1/2 second
			if (((double)(now-prev)/CLOCKS_PER_SEC)>0.5)
			{
				prev=now;

				if (Fl::ready()) {
					// refresh
					pUI->m_traceGlWindow->refresh();
					// check event
					Fl::check();

					if (Fl::damage()) {
						Fl::flush();
					}
				}
			}

			is_all_joined = true;
			for (const auto &w : workers)
			{
				if (w.wait_for(chrono::milliseconds(100))
						!= future_status::ready)
				{
					is_all_joined = false;
				}
			}
		} while (!is_all_joined);

		done=true;
		pUI->m_traceGlWindow->refresh();

		// Restore the window label
		pUI->m_traceGlWindow->label(old_label);
	}
}

void TraceUI::cb_stop(Fl_Widget* o, void* v)
{
	done=true;
}

void TraceUI::show()
{
	m_mainWindow->show();
}

void TraceUI::setRayTracer(RayTracer *tracer)
{
	raytracer = tracer;
	m_traceGlWindow->setRayTracer(tracer);
}

// menu definition
Fl_Menu_Item TraceUI::menuitems[] = {
	{ "&File",		0, 0, 0, FL_SUBMENU },
		{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)TraceUI::cb_load_scene },
		{ "&Save Image...",	FL_ALT + 's', (Fl_Callback *)TraceUI::cb_save_image },
		{ "&Exit",			FL_ALT + 'e', (Fl_Callback *)TraceUI::cb_exit },
		{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
		{ "&About",	FL_ALT + 'a', (Fl_Callback *)TraceUI::cb_about },
		{ 0 },

	{ 0 }
};

TraceUI::TraceUI() {
	// init.
	m_nDepth = 0;
	m_nSize = 150;
	m_is_enable_shadow = true;
	m_is_enable_soft_shadow = false;
	m_is_enable_reflection = true;
	m_glossy_reflection_sample = 0;
	m_is_enable_fresnel = false;
	m_fresnel_ratio = 1.0;
	m_is_enable_refraction = true;
	m_thread = 2;
	m_intensity_threshold = 0.01;
	m_super_sampling = 0;
	m_is_overide_distance = false;
	m_distance_constant = 0.25;
	m_distance_linear = 0.05;
	m_distance_quadratic = 0.01;
	m_mainWindow = new Fl_Window(100, 40, 420, 430, "Ray <Not Loaded>");
		m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
		// install menu bar
		m_menubar = new Fl_Menu_Bar(0, 0, 420, 25);
		m_menubar->menu(menuitems);

		// install slider depth
		m_depthSlider = new Fl_Value_Slider(10, 30, 260, 20, "Depth");
		m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_depthSlider->type(FL_HOR_NICE_SLIDER);
        m_depthSlider->labelfont(FL_COURIER);
        m_depthSlider->labelsize(12);
		m_depthSlider->minimum(0);
		m_depthSlider->maximum(10);
		m_depthSlider->step(1);
		m_depthSlider->value(m_nDepth);
		m_depthSlider->align(FL_ALIGN_RIGHT);
		m_depthSlider->callback(cb_depthSlides);

		// install slider size
		m_sizeSlider = new Fl_Value_Slider(10, 55, 260, 20, "Size");
		m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_sizeSlider->type(FL_HOR_NICE_SLIDER);
        m_sizeSlider->labelfont(FL_COURIER);
        m_sizeSlider->labelsize(12);
		m_sizeSlider->minimum(64);
		m_sizeSlider->maximum(512);
		m_sizeSlider->step(1);
		m_sizeSlider->value(m_nSize);
		m_sizeSlider->align(FL_ALIGN_RIGHT);
		m_sizeSlider->callback(cb_sizeSlides);

		m_shadowSwitch = new Fl_Light_Button(10, 80, 260, 20, "Shadow On");
		m_shadowSwitch->user_data((void*)(this));
		m_shadowSwitch->value(1);
		m_shadowSwitch->callback(cb_shadowSwitch);

		m_softShadowSwitch = new Fl_Light_Button(10, 105, 260, 20, "Soft Shadow On");
		m_softShadowSwitch->user_data((void*)(this));
		m_softShadowSwitch->value(0);
		m_softShadowSwitch->callback(cb_softShadowSwitch);

		m_reflectionSwitch = new Fl_Light_Button(10, 130, 260, 20, "Reflection On");
		m_reflectionSwitch->user_data((void*)(this));
		m_reflectionSwitch->value(1);
		m_reflectionSwitch->callback(cb_reflectionSwitch);

		m_glossyReflectionSlider = new Fl_Value_Slider(10, 155, 260, 20,
				"Glossy Sample");
		m_glossyReflectionSlider->user_data((void*)(this));
		m_glossyReflectionSlider->type(FL_HOR_NICE_SLIDER);
		m_glossyReflectionSlider->labelfont(FL_COURIER);
		m_glossyReflectionSlider->labelsize(12);
		m_glossyReflectionSlider->minimum(0);
		m_glossyReflectionSlider->maximum(100);
		m_glossyReflectionSlider->step(1);
		m_glossyReflectionSlider->value(m_glossy_reflection_sample);
		m_glossyReflectionSlider->align(FL_ALIGN_RIGHT);
		m_glossyReflectionSlider->callback(cb_glossyReflectionSlides);

		/*
		m_fresnelSwitch = new Fl_Light_Button(10, 180, 260, 20, "Fresnel On");
		m_fresnelSwitch->user_data((void*)(this));
		m_fresnelSwitch->value(0);
		m_fresnelSwitch->callback(cb_fresnelSwitch);

		m_fresnelSlider = new Fl_Value_Slider(10, 205, 260, 20, "Fresnel On");
		m_fresnelSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_fresnelSlider->type(FL_HOR_NICE_SLIDER);
		m_fresnelSlider->labelfont(FL_COURIER);
		m_fresnelSlider->labelsize(12);
		m_fresnelSlider->minimum(0);
		m_fresnelSlider->maximum(100);
		m_fresnelSlider->step(1);
		m_fresnelSlider->value(100);
		m_fresnelSlider->align(FL_ALIGN_RIGHT);
		m_fresnelSlider->callback(cb_fresnelSlides);
		*/
		m_refractionSwitch = new Fl_Light_Button(10, 205, 260, 20, "Refraction On");
		m_refractionSwitch->user_data((void*)(this));
		m_refractionSwitch->value(1);
		m_refractionSwitch->callback(cb_refractionSwitch);

		m_threadSlider = new Fl_Value_Slider(10, 230, 260, 20, "Thread");
		m_threadSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_threadSlider->type(FL_HOR_NICE_SLIDER);
		m_threadSlider->labelfont(FL_COURIER);
		m_threadSlider->labelsize(12);
		m_threadSlider->minimum(1);
		m_threadSlider->maximum(8);
		m_threadSlider->step(1);
		m_threadSlider->value(m_thread);
		m_threadSlider->align(FL_ALIGN_RIGHT);
		m_threadSlider->callback(cb_threadSlides);

		m_intensityThresholdSlider = new Fl_Value_Slider(10, 255, 260, 20,
				"Intensity Threshold");
		m_intensityThresholdSlider->user_data((void*)(this));
		m_intensityThresholdSlider->type(FL_HOR_NICE_SLIDER);
		m_intensityThresholdSlider->labelfont(FL_COURIER);
		m_intensityThresholdSlider->labelsize(12);
		m_intensityThresholdSlider->minimum(0.0);
		m_intensityThresholdSlider->maximum(1.0);
		m_intensityThresholdSlider->step(0.005);
		m_intensityThresholdSlider->value(m_intensity_threshold);
		m_intensityThresholdSlider->align(FL_ALIGN_RIGHT);
		m_intensityThresholdSlider->callback(cb_intensityThresholdSlides);

		m_superSamplingSlider = new Fl_Value_Slider(10, 280, 260, 20,
				"Super Sampling");
		m_superSamplingSlider->user_data((void*)(this));
		m_superSamplingSlider->type(FL_HOR_NICE_SLIDER);
		m_superSamplingSlider->labelfont(FL_COURIER);
		m_superSamplingSlider->labelsize(12);
		m_superSamplingSlider->minimum(0);
		m_superSamplingSlider->maximum(16);
		m_superSamplingSlider->step(1);
		m_superSamplingSlider->value(m_super_sampling);
		m_superSamplingSlider->align(FL_ALIGN_RIGHT);
		m_superSamplingSlider->callback(cb_superSamplingSlides);

		m_distanceSwitch = new Fl_Light_Button(10, 305, 260, 20,
				"Distance Overide");
		m_distanceSwitch->user_data((void*)(this));
		m_distanceSwitch->value(m_is_overide_distance);
		m_distanceSwitch->callback(cb_distanceSwitch);

		m_distanceConstantSlider = new Fl_Value_Slider(10, 330, 260, 20,
				"Dist. Constant");
		m_distanceConstantSlider->user_data((void*)(this));
		m_distanceConstantSlider->type(FL_HOR_NICE_SLIDER);
		m_distanceConstantSlider->labelfont(FL_COURIER);
		m_distanceConstantSlider->labelsize(12);
		m_distanceConstantSlider->minimum(0.0);
		m_distanceConstantSlider->maximum(1.0);
		m_distanceConstantSlider->step(0.005);
		m_distanceConstantSlider->value(m_distance_constant);
		m_distanceConstantSlider->align(FL_ALIGN_RIGHT);
		m_distanceConstantSlider->callback(cb_distanceConstantSlides);

		m_distanceLinearSlider = new Fl_Value_Slider(10, 355, 260, 20,
				"Dist. Linear");
		m_distanceLinearSlider->user_data((void*)(this));
		m_distanceLinearSlider->type(FL_HOR_NICE_SLIDER);
		m_distanceLinearSlider->labelfont(FL_COURIER);
		m_distanceLinearSlider->labelsize(12);
		m_distanceLinearSlider->minimum(0.0);
		m_distanceLinearSlider->maximum(1.0);
		m_distanceLinearSlider->step(0.005);
		m_distanceLinearSlider->value(m_distance_linear);
		m_distanceLinearSlider->align(FL_ALIGN_RIGHT);
		m_distanceLinearSlider->callback(cb_distanceLinearSlides);

		m_distanceQuadraticSlider = new Fl_Value_Slider(10, 380, 260, 20,
				"Dist. Quadratic");
		m_distanceQuadraticSlider->user_data((void*)(this));
		m_distanceQuadraticSlider->type(FL_HOR_NICE_SLIDER);
		m_distanceQuadraticSlider->labelfont(FL_COURIER);
		m_distanceQuadraticSlider->labelsize(12);
		m_distanceQuadraticSlider->minimum(0.0);
		m_distanceQuadraticSlider->maximum(1.0);
		m_distanceQuadraticSlider->step(0.005);
		m_distanceQuadraticSlider->value(m_distance_quadratic);
		m_distanceQuadraticSlider->align(FL_ALIGN_RIGHT);
		m_distanceQuadraticSlider->callback(cb_distanceQuadraticSlides);

		m_renderButton = new Fl_Button(340, 27, 70, 25, "&Render");
		m_renderButton->user_data((void*)(this));
		m_renderButton->callback(cb_render);

		m_stopButton = new Fl_Button(340, 55, 70, 25, "&Stop");
		m_stopButton->user_data((void*)(this));
		m_stopButton->callback(cb_stop);

		m_mainWindow->callback(cb_exit2);
		m_mainWindow->when(FL_HIDE);
    m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, "Rendered Image");
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);
}
