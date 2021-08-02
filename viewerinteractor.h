#ifndef VIEWERINTERACTOR_H
#define VIEWERINTERACTOR_H

#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkImageViewer2.h>

#include "globalstate.h"

class ViewerInteractor : public vtkInteractorStyleImage {
public:
    static			ViewerInteractor* New();
					ViewerInteractor();
					~ViewerInteractor();
                
    vtkTypeMacro(ViewerInteractor, vtkInteractorStyleImage);

	void			set_view_name(ViewName name) { view_name_ = name; }
	void			set_global_state(GlobalState* state) { global_state_ = state; }
	void			set_image_viewer(vtkSmartPointer<vtkImageViewer2> viewer) { image_viewer_ = viewer; }

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
	ViewName							view_name_;
	GlobalState*						global_state_ = nullptr;
	vtkSmartPointer<vtkImageViewer2>	image_viewer_ = nullptr;
};

#endif // VIEWERINTERACTOR_H
