#ifndef GUI_DISPLAY_WIDGETS_BRAT_VIEWS_H
#define GUI_DISPLAY_WIDGETS_BRAT_VIEWS_H

#include "new-gui/Common/QtStringUtils.h"

#include "DataModels/PlotData/ZFXYPlotData.h"
#include "DataModels/PlotData/XYPlotData.h"
#include "DataModels/PlotData/WorldPlotData.h"
#include "DataModels/PlotData/WorldPlot.h"

#include "MapWidget.h"
#include "3DPlotWidget.h"
#include "2DPlotWidget.h"

#include "ApplicationLogger.h"



 //abstract traits for views specialization
//
//	PLOT is the top v3 data class, inheriting CPlotBase
//
//	widget_type _		the GUI view's base class; the view supplies it's specialized data to this base class
//	properties_type -	brat v3 specialized "property" class: a set of view properties
//	data_type -			v3 class with the processed data, that makes the link to vtk
//
template< class PLOT >
struct plot_traits;


// traits for map view specialization
//
template<>
struct plot_traits< CWPlot >
{
	using properties_type = CWorldPlotProperties;
	using data_type = CWorldPlotData;
};


// traits for 3D view specialization
//
template<>
struct plot_traits< CZFXYPlot >
{
	using properties_type = CZFXYPlotProperties;
	using data_type = CZFXYPlotData;
};


// traits for 2D view specialization
//
template<>
struct plot_traits< CPlot >
{
	using properties_type = CXYPlotProperties;
	using data_type = CXYPlotData;
};







template< class WIDGET >
class CBratView : public WIDGET
{
	//types

	using base_t = WIDGET;

public:
	using base_t::setWindowTitle;


protected:

	//construction / destruction

	//for maps with raster option
	//
    template< typename OPTION >
    CBratView( OPTION option, QWidget *parent )
        : base_t( option, parent )
	{}
public:
    CBratView( QWidget *parent = nullptr ) 
		: base_t( parent )
	{}

	virtual ~CBratView()
	{}

	//instance member functions

	virtual void CreatePlot( const CWorldPlotProperties *props, CWPlot *plot );

protected:

	virtual void AddData( CWorldPlotData *pdata, size_t index ) = 0;
};





template< class WIDGET >
void CBratView< WIDGET >::CreatePlot( const CWorldPlotProperties *props, CWPlot* wplot )
{
	//wplot->GetInfo();		 assumed done by caller

    Q_UNUSED( props );  //mPlotPropertiesMap = *props;	// *proc->GetWorldPlotProperties( 0 );
	this->setWindowTitle( t2q( wplot->MakeTitle() ) );

	// for Geo-strophic velocity
	//
	CPlotField * northField =nullptr;
	CPlotField * eastField =nullptr;

	size_t index = 0;
	for ( auto &itField : wplot->m_fields )
	{
		CPlotField* field = CPlotField::GetPlotField( itField );

#if defined(_DEBUG) || defined(DEBUG) 
		if ( field->m_worldProps != props )
			LOG_WARN( "field->m_worldProps != props" );
#endif

		if ( field->m_internalFiles.empty() )
			continue;

		if ( field->m_worldProps->m_northComponent && northField == nullptr ) 
		{
			northField = field;
			continue;
		}
		else
		if ( field->m_worldProps->m_eastComponent && eastField == nullptr ) 
		{
			eastField = field;
			continue;
		}

		// otherwise just add it as regular data
		AddData( new CWorldPlotData( field ), index++ );					   		//v4 note: CWorldPlotData ctor is only invoked here
	}

	// we have a Vector Plot!
	if ( northField != nullptr && eastField != nullptr ) 
	{
		AddData( new CWorldPlotVelocityData( northField, eastField ), 0 );	//v4 note: CWorldPlotVelocityData ctor is only invoked here
	}
	else if ( northField != eastField ) 
	{
		CException e( "CBratDisplayApp::CreateWPlot - incomplete std::vector plot components", BRATHL_INCONSISTENCY_ERROR );
		LOG_TRACE( e.what() );
		throw e;
	}
}



//template< class WIDGET >
//void CBratView< WIDGET >::CreatePlot( const CZFXYPlotProperties *props, CZFXYPlot *zfxyplot, bool histogram )
//{
//	//zfxyplot->GetInfo();	 assumed done by caller
//
//    Q_UNUSED( props );  //mPlotProperties3D = *props;	// *proc->GetZFXYPlotProperties( 0 );
//	this->setWindowTitle( t2q( zfxyplot->MakeTitle() ) );
//
//	//CZFXYPlotFrame* frame = new CZFXYPlotFrame( nullptr, -1, title, zfxyPlotProperty, pos, size );
//
//	size_t index = 0;
//	for ( auto &itField : zfxyplot->m_fields )
//	{
//		CPlotField* field = CPlotField::GetPlotField( itField );
//
//#if defined(_DEBUG) || defined(DEBUG) 
//		if( field->m_zfxyProps != props ) 
//			LOG_WARN( "field->m_worldProps != props" );
//#endif
//
//		if ( field->m_internalFiles.empty() )
//			continue;
//
//		//frame->AddData( geoMap );
//		AddData( new CZFXYPlotData( zfxyplot, field ), index++, histogram );	// v4 note: CZFXYPlotData ctor only invoked here
//	}
//
//	//frame->UpdateView();
//	//frame->Raise();
//	//frame->Show( TRUE );
//
//}



//
//	CXYPlotData		is a	CBratObject							
//					has a	vtkDataArrayPlotData
//								has a vtkDoubleArrayBrathl *XDataArray;
//								has a vtkDoubleArrayBrathl *YDataArray;						
//								has a vtkDoubleArrayBrathl *ZDataArray;						
//										
//	CPlotArray	has a 	vtkObArray (array of frames)
//
//
//	CXYPlotData		=> Create	=> SetData( 
//	(has a vtkDataArrayPlotData)			double* xArray, const CUIntArray& xDims, 
//											double* yArray, const CUIntArray& yDims,
//											const CUIntArray& xCommonDimIndex,
//											const CUIntArray& yCommonDimIndex ) => ...
//
//	... => CPlotArray::SetFrameData for each frame, 
//		and at the end:
//	... => SetRawData( CPlotArray* x, CPlotArray* y ) => Update() => m_vtkDataArrayPlotData::SetDataArrays( vtkDoubleArrayBrathl, vtkDoubleArrayBrathl )
//
//template< class WIDGET >
//void CBratView< WIDGET >::CreatePlot( const CXYPlotProperties *props, CPlot *plot, bool histogram )
//{
//	// v4 taken from CBratDisplayApp::CreateXYPlot(CPlot* plot, wxSize& size, wxPoint& pos) and callees
//
//    //plot->GetInfo(); 	 assumed done by caller
//
//    Q_UNUSED( props );  //mPlotProperties2D = *props;		// *proc->GetXYPlotProperty( 0 );		v4: nothing similar to this, similar to zfxy and lat - lon, was done here
//
//	setWindowTitle( t2q( plot->MakeTitle() ) );
//
//	//CXYPlotFrame* frame = new CXYPlotFrame( nullptr, -1, title, pos, size );
//
//	int nrFields = (int)plot->m_fields.size();
//
//	for ( int iField = 0; iField < nrFields; iField++ )
//	{
//		//auto field = plot->GetPlotField(iField);
//		//assert__(field->m_xyProps == props );
//
//		//frame->AddData( plotData );
//		AddData( new CXYPlotData( plot, iField ), iField, histogram );		//v4 note: CXYPlotData ctor only invoked here
//	}
//
//	//frame->Raise();
//
//	//frame->ShowPropertyPanel();
//	//frame->Show( TRUE );
//
//	//pos = frame->GetPosition();
//
//}










//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//									CBratMapView
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


class CBratMapView : public CBratView< CMapWidget >
{
#if defined (__APPLE__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif

    Q_OBJECT;

#if defined (__APPLE__)
#pragma clang diagnostic pop
#endif

	//types

    using base_t = CBratView< CMapWidget >;


	// instance data

	std::vector< CWorldPlotData* > mMapDataCollection;


	// construction /destruction

public:
    CBratMapView( QWidget *parent ) 
		: base_t( parent )
	{}

    CBratMapView( CMapWidget::ELayerBaseType layer_type, QWidget *parent )
        : base_t( layer_type, parent )
	{}

	virtual ~CBratMapView()
	{}


	//access

	std::vector< CWorldPlotData* >* MapDataCollection() { return &mMapDataCollection; }


	//operations

	// CBratView interface

protected:
	void Plot( const CWorldPlotInfo &maps, CWorldPlotProperties *props );

	virtual void AddData( CWorldPlotData *pdata, size_t index ) override;
};






#endif			// GUI_DISPLAY_WIDGETS_BRAT_VIEWS_H
