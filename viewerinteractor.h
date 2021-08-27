#ifndef VIEWERINTERACTOR_H
#define VIEWERINTERACTOR_H

#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkImageViewer2.h>
#include <vtkImageData.h>
#include <vtkCoordinate.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>

#include <functional>

#include "globalstate.h"

enum class Event {
    MOVE_SLICE_PLUS,	MOVE_SLICE_MINUS,	DRAG_SLICE,
	ZOOM_IN,			ZOOM_OUT,			
	DRAW_ROI,           ADD_NODULE
};

class ViewerInteractor : public vtkInteractorStyleImage {
public:
    static			ViewerInteractor* New();
					ViewerInteractor() = default;
					~ViewerInteractor();
                
    vtkTypeMacro(ViewerInteractor, vtkInteractorStyleImage);

	void			set_view_name(ViewName name) { view_name_ = name; }
	void			set_global_state(GlobalState* state) { global_state_ = state; }
	void			set_image_viewer(vtkImageViewer2* viewer) { image_viewer_ = viewer; }
    
    void			AddEvent(Event event_name, std::function<void()> func) { event_map_[event_name] = func; }
    
protected:
	virtual void	OnMouseMove() override;
	virtual void    OnMouseWheelForward() override;
	virtual void    OnMouseWheelBackward() override;
	virtual void	OnLeftButtonDown() override;
	virtual void	OnLeftButtonUp() override;
	virtual void	OnRightButtonDown() override;
	virtual void	OnRightButtonUp() override;
	virtual void	OnKeyDown() override;
	virtual void	OnKeyUp() override;
    
private:
    void            ConfirmCurrentControlView();
	double*			GetMouseWorldLocation();

private:
	ViewName                                view_name_ = ViewName::TRA;
	GlobalState*                            global_state_ = nullptr;
	vtkImageViewer2*                        image_viewer_ = nullptr;
	vtkSmartPointer<vtkCoordinate>			coordinate_ = nullptr;

    std::map<Event, std::function<void()>>  event_map_;
};

#endif // VIEWERINTERACTOR_H
