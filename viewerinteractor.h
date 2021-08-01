#ifndef VIEWERINTERACTOR_H
#define VIEWERINTERACTOR_H

#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>

class ViewerInteractor : public vtkInteractorStyleImage {
public:
    static      ViewerInteractor* New();
                ViewerInteractor();
                ~ViewerInteractor();
                
    vtkTypeMacro(ViewerInteractor, vtkInteractorStyleImage);
    
};

#endif // VIEWERINTERACTOR_H
