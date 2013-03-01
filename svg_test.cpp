#include <stdio.h>
#include <stdlib.h>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgba.h"
#include "platform/agg_platform_support.h"
#include "ctrl/agg_slider_ctrl.h"
#include "agg_svg_parser.h"

enum { flip_y = false };


class the_application : public agg::platform_support
{
    agg::svg::path_renderer m_path;

    double m_min_x;
    double m_min_y;
    double m_max_x;
    double m_max_y;

    double m_x;
    double m_y;
    double m_dx;
    double m_dy;
    bool   m_drag_flag;

	double m_scale;

public:

    the_application(agg::pix_format_e format, bool flip_y) :
        agg::platform_support(format, flip_y),
        m_path(),
        m_min_x(0.0),
        m_min_y(0.0),
        m_max_x(0.0),
        m_max_y(0.0),
        m_x(0.0),
        m_y(0.0),
        m_dx(0.0),
        m_dy(0.0),
        m_drag_flag(false),
		m_scale(1.0)
    {
    }

    void parse_svg(const char* fname)
    {
        agg::svg::parser p(m_path);
        p.parse(fname);
        m_path.arrange_orientations();
        m_path.bounding_rect(&m_min_x, &m_min_y, &m_max_x, &m_max_y);
        caption(p.title());
    }

    virtual void on_resize(int cx, int cy)
	{
		double w = m_max_x-m_min_x;
		double h = m_max_y-m_min_y;
		double scale_x = (cx-50) / w;
		double scale_y = (cy-50) / h;
		m_scale = (scale_x < scale_y) ? scale_x : scale_y;
		m_x = cx / 2;
		m_y = (cy-50) / 2;
    }

    virtual void on_draw()
    {
        typedef agg::pixfmt_bgra32 pixfmt;
        typedef agg::renderer_base<pixfmt> renderer_base;
        typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

        pixfmt pixf(rbuf_window());
        renderer_base rb(pixf);
        renderer_solid ren(rb);

        rb.clear(agg::rgba(1,1,1));

        agg::rasterizer_scanline_aa<> ras;
        agg::scanline_p8 sl;
        agg::trans_affine mtx;

        mtx *= agg::trans_affine_translation((m_min_x + m_max_x) * -0.5, (m_min_y + m_max_y) * -0.5);
        mtx *= agg::trans_affine_scaling(m_scale);
        mtx *= agg::trans_affine_translation((m_min_x + m_max_x) * 0.5 + m_x, (m_min_y + m_max_y) * 0.5 + m_y + 30);
        
        start_timer();
        m_path.render(ras, sl, ren, mtx, rb.clip_box(), 1.0);
        double tm = elapsed_time();
        unsigned vertex_count = m_path.vertex_count();
		
        char buf[128]; 
        agg::gsv_text t;
        t.size(8.0);
        t.flip(true);

        agg::conv_stroke<agg::gsv_text> pt(t);
        pt.width(1.5);

        sprintf(buf, "Scale=%2.1f Time=%.3f ms", m_scale, tm);

        t.start_point(10.0, 20.0);
        t.text(buf);

        ras.add_path(pt);
        ren.color(agg::rgba(0,0,0));
        agg::render_scanlines(ras, sl, ren);

    }

    virtual void on_mouse_button_down(int x, int y, unsigned flags)
    {
        m_dx = x - m_x;
        m_dy = y - m_y;
        m_drag_flag = true;
    }

    virtual void on_mouse_move(int x, int y, unsigned flags)
    {
        if(flags == 0)
        {
            m_drag_flag = false;
        }

        if(m_drag_flag)
        {
            m_x = x - m_dx;
            m_y = y - m_dy;
            force_redraw();
        }
    }

    virtual void on_mouse_button_up(int x, int y, unsigned flags)
    {
        m_drag_flag = false;
    }

    virtual void on_key(int x, int y, unsigned key, unsigned flags)
    {
        if(key == 'a')
		{
			if (m_scale < 50)
				m_scale *= 1.2;
			force_redraw();
		}
		else if(key == 's')
		{
			if (m_scale > 0.5)
				m_scale *= 0.8;
			force_redraw();
		}
    }



};




int agg_main(int argc, char* argv[])
{
    the_application app(agg::pix_format_bgra32, flip_y);

    const char* fname = "tiger.svg";
    if(argc <= 1)
    {
        FILE* fd = fopen(app.full_file_name(fname), "r");
        if(fd == 0)
        {
            app.message("Usage: svg_test <svg_file>\n"
                        "Download http://antigrain.com/svg/tiger.svg");
            return 1;
        }
        fclose(fd);
    }
    else
    {
        fname = argv[1];
    }

    try
    {
        app.parse_svg(app.full_file_name(fname));
    }
    catch(agg::svg::exception& e)
    {
        app.message(e.msg());
        return 1;
    }

    if(app.init(600, 480, agg::window_resize))
    {
        return app.run();
    }
    return 1;
}



