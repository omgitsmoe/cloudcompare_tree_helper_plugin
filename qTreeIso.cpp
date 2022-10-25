//##########################################################################
//#                                                                        #
//#                CLOUDCOMPARE PLUGIN: qTreeIso                           #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#                      COPYRIGHT: Morris Aurich                          #
//#                                                                        #
//##########################################################################

// First:
//	Replace all occurrences of 'ExamplePlugin' by your own plugin class name in this file.
//	This includes the resource path to info.json in the constructor.

// Second:
//	Open ExamplePlugin.qrc, change the "prefix" and the icon filename for your plugin.
//	Change the name of the file to <yourPluginName>.qrc

// Third:
//	Open the info.json file and fill in the information about the plugin.
//	 "type" should be one of: "Standard", "GL", or "I/O" (required)
//	 "name" is the name of the plugin (required)
//	 "icon" is the Qt resource path to the plugin's icon (from the .qrc file)
//	 "description" is used as a tootip if the plugin has actions and is displayed in the plugin dialog
//	 "authors", "maintainers", and "references" show up in the plugin dialog as well

// STL
#include <vector>
#include <map>
#include <algorithm>

// Qt
#include <QtGui>

// common
#include <ccPickingHub.h>

//qCC_db
#include <ccHObjectCaster.h>
#include <ccPointCloud.h>
#include <ccGenericPointCloud.h>
// #include <CCConst.h>

// qCC

// CCLib
#include <ReferenceCloud.h>


#include "qTreeIso.h"

// Default constructor:
//	- pass the Qt resource path to the info.json file (from <yourPluginName>.qrc file) 
//  - constructor should mainly be used to initialize actions and other members
qTreeIso::qTreeIso( QObject *parent )
	: QObject( parent )
	, ccStdPluginInterface( ":/CC/plugin/qTreeIso/info.json" )
	, m_actionSplitByColor( nullptr )
	, m_actionSplitByScalar( nullptr )
	, m_actionRemoveLowestPoints( nullptr )
	, m_actionBHDSlice( nullptr )
	, m_actionExtractPointsOfSameColor( nullptr )
	, m_actionExtractPointsOfSameColorIntoOther( nullptr )
{
}

// This method should enable or disable your plugin actions
// depending on the currently selected entities ('selectedEntities').
void qTreeIso::onNewSelection( const ccHObject::Container &selectedEntities )
{
	if ( m_actionSplitByColor == nullptr )
	{
		return;
	}
	
	// If you need to check for a specific type of object, you can use the methods
	// in ccHObjectCaster.h or loop and check the objects' classIDs like this:
	//
	//	for ( ccHObject *object : selectedEntities )
	//	{
	//		if ( object->getClassID() == CC_TYPES::VIEWPORT_2D_OBJECT )
	//		{
	//			// ... do something with the viewports
	//		}
	//	}
	
	// For example - only enable our action if something is selected.
	bool selected = !selectedEntities.empty();
	m_actionSplitByColor->setEnabled( selected );
	m_actionSplitByScalar->setEnabled( selected );
	m_actionRemoveLowestPoints->setEnabled( selected );
	m_actionExtractPointsOfSameColorIntoOther->setEnabled( selected );
}

// This method returns all the 'actions' your plugin can perform.
// getActions() will be called only once, when plugin is loaded.
QList<QAction *> qTreeIso::getActions()
{
	// default action (if it has not been already created, this is the moment to do it)
	// always initialized at once
	if ( !m_actionSplitByColor )
	{
		m_actionSplitByColor = new QAction( QString("Split by color"), this );
		m_actionSplitByColor->setToolTip( QString("Split cloud by rgb color") );
		m_actionSplitByColor->setIcon( getIcon() );
		
		// Connect appropriate signal
		connect( m_actionSplitByColor, &QAction::triggered, this, &qTreeIso::doSplitByColor );

		// ACTION split by scalar
		m_actionSplitByScalar = new QAction( QString("Split by scalar"), this );
		m_actionSplitByScalar->setToolTip( QString("Split cloud by active scalar field") );
		m_actionSplitByScalar->setIcon( getIcon() );
		connect( m_actionSplitByScalar, &QAction::triggered, this, &qTreeIso::doSplitByScalar );

		// ACTION remove lowest pts
		m_actionRemoveLowestPoints = new QAction( QString("Remove lowest points"), this );
		m_actionRemoveLowestPoints->setToolTip( QString("Removes lowest points in cloud based on a 2d grid") );
		m_actionRemoveLowestPoints->setIcon( getIcon() );
		connect( m_actionRemoveLowestPoints, &QAction::triggered, this, &qTreeIso::doRemoveLowestPoints );

		m_actionBHDSlice = new QAction( QString("BHD Slice"), this );
		m_actionBHDSlice->setToolTip( QString("Select a point at the start of the trunk (corrected for slope etc.) that you want a BHD slice of") );
		m_actionBHDSlice->setIcon( getIcon() );
		connect( m_actionBHDSlice, &QAction::triggered, this, &qTreeIso::doBHDSlice );

		// ACTION remove points of same color
		m_actionExtractPointsOfSameColor = new QAction( QString("Extract points of same color!"), this );
		m_actionExtractPointsOfSameColor->setToolTip( QString("Pick a point in a point cloud and extract all points in that cloud that have the same color (points in old cloud are deleted)!") );
		m_actionExtractPointsOfSameColor->setIcon( getIcon() );
		connect(m_actionExtractPointsOfSameColor, &QAction::triggered, this, &qTreeIso::doExtractPointsOfSameColor);

		m_actionExtractPointsOfSameColorIntoOther = new QAction( QString("Extract points of same color!"), this );
		m_actionExtractPointsOfSameColorIntoOther->setToolTip( QString("Pick a point in a point cloud and extract all points in that cloud that have the same color (points in old cloud are deleted) into selected entity (in DB Tree)!") );
		m_actionExtractPointsOfSameColorIntoOther->setIcon( getIcon() );
		connect(m_actionExtractPointsOfSameColorIntoOther, &QAction::triggered, this, &qTreeIso::doExtractPointsOfSameColorIntoOther);
	}

	return { m_actionSplitByColor, m_actionSplitByScalar, m_actionRemoveLowestPoints, m_actionBHDSlice, m_actionExtractPointsOfSameColor, m_actionExtractPointsOfSameColorIntoOther };
}

// This is an example of an action's method called when the corresponding action
// is triggered (i.e. the corresponding icon or menu entry is clicked in CC's
// main interface). You can access most of CC's components (database,
// 3D views, console, etc.) via the 'm_app' variable (see the ccMainAppInterface
// class in ccMainAppInterface.h).
void qTreeIso::doExtractPointsOfSameColor()
{
	m_onPointPicked = EXTRACT_POINTS;
	startPicking();
}

void qTreeIso::doExtractPointsOfSameColorIntoOther()
{
	m_onPointPicked = EXTRACT_POINTS_INTO_OHTER;
	startPicking();
}

// adapted from ccCompass plugin
//registers this plugin with the picking hub
bool qTreeIso::startPicking()
{
	if (m_picking) //already picking... don't need to add again
		return true;

	//check valid gl window
	if (!m_app->getActiveGLWindow())
	{
		//invalid pointer error
		m_app->dispToConsole("Error: qTreeIso could not find the Cloud Compare window. Abort!", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
		return false;
	}

	//activate "point picking mode"
	if (!m_app->pickingHub())  //no valid picking hub
	{
		m_app->dispToConsole("[qTreeIso] Could not retrieve valid picking hub. Measurement aborted.", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
		return false;
	}

	if (!m_app->pickingHub()->addListener(this, true, true))
	{
		m_app->dispToConsole("Another tool is already using the picking mechanism. Stop it first", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
		return false;
	}

	m_picking = true;
	return true;
}

//removes this plugin from the picking hub
void  qTreeIso::stopPicking()
{
	//stop picking
	if (m_app->pickingHub())
	{
		m_app->pickingHub()->removeListener(this);
	}

	m_picking = false;
}

//This function is called when a point is picked (through the picking hub)
void qTreeIso::onItemPicked(const ccPickingListener::PickedItem& pi)
{
	switch (m_onPointPicked)
	{
	case NONE:
		return;
	case EXTRACT_POINTS:
		pointPickedExtractPointsOfSameColor(pi.entity, pi.itemIndex, pi.clickPoint.x(), pi.clickPoint.y(), pi.P3D); //map straight to pointPicked function
		break;
	case EXTRACT_POINTS_INTO_OHTER:
		pointPickedExtractPointsOfSameColorIntoOther(pi.entity, pi.itemIndex, pi.clickPoint.x(), pi.clickPoint.y(), pi.P3D);
		break;
	case DO_BHD_SLICE:
		pointPickedBHDSlice(pi.entity, pi.itemIndex, pi.clickPoint.x(), pi.clickPoint.y(), pi.P3D);
		break;
	}
}

void qTreeIso::pointPickedExtractPointsOfSameColor(ccHObject* entity, unsigned itemIdx, int x, int y, const CCVector3& P)
{
	if (!entity) //null pick
	{
		return;
	}

	////no active tool (i.e. picking mode) - set selected object as active
	//if (!m_activeTool)
	//{
	//	m_app->setSelectedInDB(entity, true);
	//	return;
	//}

	// make sure we picked a point cloud?
	if (!entity->isKindOf(CC_TYPES::POINT_CLOUD))
	{
		m_app->dispToConsole( "[qTreeIso] Warning: Please select a point that's part of a point cloud !", ccMainAppInterface::WRN_CONSOLE_MESSAGE );
		return;
	}
	//get point cloud
	// ccGenericPointCloud* genCloud = ccHObjectCaster::ToGenericPointCloud(entity);
	ccPointCloud* cloud = static_cast<ccPointCloud*>(entity); //cast to point cloud

	if (!cloud)
	{
		ccLog::Warning("[Item picking] Shit's fubar (Picked point is not in pickable entities DB?)!");
		return;
	}

	auto pointColor = cloud->getPointColor(itemIdx);
	m_app->dispToConsole(QString("[qTreeIso] Picked point's color is: R %1 G %2 B %3").arg(pointColor.r).arg(pointColor.g).arg(pointColor.b), ccMainAppInterface::STD_CONSOLE_MESSAGE);

	unsigned numberOfPoints = cloud->size();
	ccHObject* parent = cloud->getParent();
	if (!parent->isGroup())
	{
		m_app->dispToConsole( "[qTreeIso] Warning: Couldn't find group belonging to point cloud !", ccMainAppInterface::WRN_CONSOLE_MESSAGE );
		return;
	}
	// CCLib::ReferenceCloud sameColorIndexes(cloud);
	// use the point cloud's default visiblity array but we have to reset it and lose current visiblity info
	// since there might be points already hidden but since we lose this information anyway when usinge createNewCloudFromVisbilitySelection?
	// otherwise we create our own separate vis table and pass that one
	if (!cloud->resetVisibilityArray()) // resets visiblity array or allocates it if it wasn't already
	{
		ccLog::Error(QString("[Cloud %1] Visibility table could not be instantiated!").arg(cloud->getName()));
		return;
	}
	ccGenericPointCloud::VisibilityTableType& visibilityArray = cloud->getTheVisibilityArray();

	ccColor::Rgb cPtColor;
	for (unsigned i = 0; i < numberOfPoints; ++i)
	{
		cPtColor = cloud->getPointColor(i);
		// default all pts start as visibilityArray[i] = POINT_VISIBLE;
		if (!((cPtColor.r == pointColor.r) &&(cPtColor.g == pointColor.g) && (cPtColor.b == pointColor.b)))
		{
			visibilityArray[i] = POINT_HIDDEN;

			// if color matches
			//if (!sameColorIndexes.addPointIndex(i))
			//{
			//	// out of memory
			//	return;
			//}
		}

	}
	// m_app->dispToConsole(QString("[qTreeIso] Found %1 points with the same color !").arg(sameColorIndexes.size()), ccMainAppInterface::STD_CONSOLE_MESSAGE);
	m_app->dispToConsole(QString("[qTreeIso] Found %1 points with the same color !").arg(std::count(visibilityArray.begin(), visibilityArray.end(), POINT_VISIBLE)), ccMainAppInterface::STD_CONSOLE_MESSAGE);

	//we temporarily detach the entity, as it may undergo
	//"severe" modifications (octree deletion, etc.) --> see ccPointCloud::createNewCloudFromVisibilitySelection
	// taken from MainWindow::deactivateSegmentationMode
	ccMainAppInterface::ccHObjectContext objContext = m_app->removeObjectTemporarilyFromDBTree(entity);

	// no easy way to delete points from cloud -> for examples see MainWindow::deactivateSegmentationMode, ccGraphicalSegementationTool.cpp, ManualSegmentationTools.cpp
	// have to use ccPointCloud::createNewCloudFromVisibilitySelection (which is also used by CC's segmentation tools see deactivateSegmentationMode)
	// OLD ccPointCloud* extractedPoints = cloud->partialClone(&sameColorIndexes);  // new cloud from reference cloud's indices
	// new cloud will be created out of visible points and if we pass removeSelectedPoints=true then the visible points get removed from the original cloud
	auto extractedPoints = cloud->createNewCloudFromVisibilitySelection(true);
	if (extractedPoints)
	{
		extractedPoints->showColors(true);
		extractedPoints->showSF(false);

		//'shift on load' information
		extractedPoints->setGlobalShift(cloud->getGlobalShift());
		extractedPoints->setGlobalScale(cloud->getGlobalScale());

		extractedPoints->setVisible(true);
		extractedPoints->setName(QString("cR%1G%2B%3").arg(pointColor.r).arg(pointColor.g).arg(pointColor.b));

		//we add new CC to parent
		parent->addChild(extractedPoints);
		m_app->addToDB(extractedPoints);

		m_app->setSelectedInDB(extractedPoints, true);
	}
	else
	{
		m_app->dispToConsole(QString("[qTreeIso:extractPointsOfSameColor] Failed to create extracted cloud ! (not enough memory)"), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
	}

	// entity->setName(entity->getName() + QString(".remaining"));
	m_app->putObjectBackIntoDBTree(entity, objContext);

	stopPicking();
	//redraw
	m_app->updateUI();
	m_app->getActiveGLWindow()->redraw();
}

//Process point picks
void qTreeIso::pointPickedExtractPointsOfSameColorIntoOther(ccHObject* entity, unsigned itemIdx, int x, int y, const CCVector3& P)
{
	if (!entity) //null pick
	{
		return;
	}
	auto selEntities = m_app->getSelectedEntities();
	// cloud that points should be inserted into must be the only selection
	// onNewSelection already makes sure we have at least one selection
	if (selEntities.size() > 1)
	{
		m_app->dispToConsole( "[qTreeIso] Warning: Please only select cloud that the points should be extracted to!", ccMainAppInterface::WRN_CONSOLE_MESSAGE );
		return;
	}

	if (entity == selEntities[0])
	{
		m_app->dispToConsole("[qTreeIso] Points are already in selected entity!", ccMainAppInterface::STD_CONSOLE_MESSAGE);
		return;
	}

	////no active tool (i.e. picking mode) - set selected object as active
	//if (!m_activeTool)
	//{
	//	m_app->setSelectedInDB(entity, true);
	//	return;
	//}

	// make sure we picked a point cloud?
	if (!entity->isKindOf(CC_TYPES::POINT_CLOUD))
	{
		m_app->dispToConsole( "[qTreeIso] Warning: Please select a point that's part of a point cloud !", ccMainAppInterface::WRN_CONSOLE_MESSAGE );
		return;
	}
	//get point cloud
	// ccGenericPointCloud* genCloud = ccHObjectCaster::ToGenericPointCloud(entity);
	ccPointCloud* cloud = static_cast<ccPointCloud*>(entity); //cast to point cloud

	if (!cloud)
	{
		ccLog::Warning("[Item picking] Shit's fubar (Picked point is not in pickable entities DB?)!");
		return;
	}

	auto pointColor = cloud->getPointColor(itemIdx);
	m_app->dispToConsole(QString("[qTreeIso] Picked point's color is: R %1 G %2 B %3").arg(pointColor.r).arg(pointColor.g).arg(pointColor.b), ccMainAppInterface::STD_CONSOLE_MESSAGE);

	unsigned numberOfPoints = cloud->size();
	ccHObject* parent = cloud->getParent();
	if (!parent->isGroup())
	{
		m_app->dispToConsole( "[qTreeIso] Warning: Couldn't find group belonging to point cloud !", ccMainAppInterface::WRN_CONSOLE_MESSAGE );
		return;
	}
	// CCLib::ReferenceCloud sameColorIndexes(cloud);
	// use the point cloud's default visiblity array but we have to reset it and lose current visiblity info
	// since there might be points already hidden but since we lose this information anyway when usinge createNewCloudFromVisbilitySelection?
	// otherwise we create our own separate vis table and pass that one
	if (!cloud->resetVisibilityArray()) // resets visiblity array or allocates it if it wasn't already
	{
		ccLog::Error(QString("[Cloud %1] Visibility table could not be instantiated!").arg(cloud->getName()));
		return;
	}
	ccGenericPointCloud::VisibilityTableType& visibilityArray = cloud->getTheVisibilityArray();

	ccColor::Rgb cPtColor;
	for (unsigned i = 0; i < numberOfPoints; ++i)
	{
		cPtColor = cloud->getPointColor(i);
		// default all pts start as visibilityArray[i] = POINT_VISIBLE;
		if (!((cPtColor.r == pointColor.r) &&(cPtColor.g == pointColor.g) && (cPtColor.b == pointColor.b)))
		{
			visibilityArray[i] = POINT_HIDDEN;

			// if color matches
			//if (!sameColorIndexes.addPointIndex(i))
			//{
			//	// out of memory
			//	return;
			//}
		}

	}
	// m_app->dispToConsole(QString("[qTreeIso] Found %1 points with the same color !").arg(sameColorIndexes.size()), ccMainAppInterface::STD_CONSOLE_MESSAGE);
	m_app->dispToConsole(QString("[qTreeIso] Found %1 points with the same color !").arg(std::count(visibilityArray.begin(), visibilityArray.end(), POINT_VISIBLE)), ccMainAppInterface::STD_CONSOLE_MESSAGE);

	//we temporarily detach the entity, as it may undergo
	//"severe" modifications (octree deletion, etc.) --> see ccPointCloud::createNewCloudFromVisibilitySelection
	// taken from MainWindow::deactivateSegmentationMode
	ccMainAppInterface::ccHObjectContext objContext = m_app->removeObjectTemporarilyFromDBTree(entity);

	/// 
	/// get cloud to insert points into
	///
	ccHObject* targetEntity = selEntities[0];
	ccGenericPointCloud* targetCloud;
	ccPointCloud* targetPC;
	targetCloud = ccHObjectCaster::ToGenericPointCloud(targetEntity);
	if (targetCloud && targetCloud->isA(CC_TYPES::POINT_CLOUD))
	{
		targetPC = static_cast<ccPointCloud*>(targetCloud);
	}
	else
	{
		m_app->dispToConsole(QString("Entity [%1] is not of type PointCloud !").arg(selEntities[0]->getName()), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
		return;
	}
	// remember these
	bool targetShowColor = targetPC->colorsShown();
	bool targetShowSF = targetPC->sfShown();
	// check if target cloud has colors -> not needed since it's handled by += operator (which calls append)
	//if (!targetPC->hasColors())
	//{
	//	m_app->dispToConsole(QString("Entity [%1] has no colors !").arg(selEntities[0]->getName()), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
	//	return;
	//}

	// no easy way to delete points from cloud -> for examples see MainWindow::deactivateSegmentationMode, ccGraphicalSegementationTool.cpp, ManualSegmentationTools.cpp
	// have to use ccPointCloud::createNewCloudFromVisibilitySelection (which is also used by CC's segmentation tools see deactivateSegmentationMode)
	// OLD ccPointCloud* extractedPoints = cloud->partialClone(&sameColorIndexes);  // new cloud from reference cloud's indices
	// new cloud will be created out of visible points and if we pass removeSelectedPoints=true then the visible points get removed from the original cloud
	// @Speed using createNewClouwFromVisiblitySelection even though we want to only extract the points into an existing cloud,
	// since it handles deleting points properly
	auto extractedPoints = cloud->createNewCloudFromVisibilitySelection(true);
	if (!extractedPoints)
	{
		m_app->dispToConsole(QString("[qTreeIso:extractPointsOfSameColor] Failed to create extracted cloud ! (not enough memory)"), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
		return;
	}

	ccPointCloud* extractedCloud;
	if (extractedPoints->isA(CC_TYPES::POINT_CLOUD))
	{
		extractedCloud = static_cast<ccPointCloud*>(extractedPoints);
	}
	else
	{
		m_app->dispToConsole(QString("Selected entity [%1] is not of type PointCloud !").arg(selEntities[0]->getName()), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
		return;
	}
	*targetPC += extractedCloud;

	// delete temp extractedPoints cloud
	delete extractedPoints;
	extractedPoints = nullptr;
	extractedCloud = nullptr;

	// reset these to the old values since the user might want to not display the color or the cloud at all
	// to be able to see extracted points better
	// otherwise i could set all the extracted point's to the cloud's color and save the original
	// color as a packed int32 scalar field
	targetPC->showColors(targetShowColor);
	targetPC->showSF(targetShowSF);
	// needs to be set on the parent! targetPC->setVisible(true);

	m_app->putObjectBackIntoDBTree(entity, objContext);

	stopPicking();
	//redraw
	m_app->updateUI();
	m_app->getActiveGLWindow()->redraw();
}

void qTreeIso::doSplitByColor()
{
	splitCloudBy(true, true, false);
}

void qTreeIso::doSplitByScalar()
{
	splitCloudBy(false, true, false);

}

float findMedian(std::vector<float>& v)
{
	// This algorithm handles both evenand odd sized inputs efficiently using
	// the STL nth_element(amortized O(N)) algorithmand the max_element
	// algorithm(O(n)).Note that nth_element has another guaranteed side
	// effect, namely that all of the elements before n are all guaranteed to
	// be less than v[n], just not necessarily sorted
	// credit: Croc Dialer stackoverflow.com/questions/1719070
	std::vector<float> tmp_array(std::begin(v), std::end(v));
	size_t n = tmp_array.size() / 2;
	// nth_element does enough of a sort to put one chosen element in the
	// correct position, but doesn't completely sort the container
	// sort till half of the array
	std::nth_element(tmp_array.begin(), tmp_array.begin() + n, tmp_array.end());

	// odd array return nth element as median
	if (tmp_array.size() % 2) { return tmp_array[n]; }
	else
	{
		// even sized vector -> average the two middle values
		// since nth_element guarantees that all elements before n are smaller
		// we can get max to find other middle val
		// we could also use nth_element again with n as v.begin()+n-1
		// but then we have to save tmp_array[n] first since nth_element will change the order
		auto max_it = std::max_element(tmp_array.begin(), tmp_array.begin() + n);
		return (*max_it + tmp_array[n]) * 0.5f;
	}
}

void qTreeIso::doRemoveLowestPoints()
{
	if (m_app == nullptr)
	{
		// m_app should have already been initialized by CC when plugin is loaded
		Q_ASSERT(false);

		return;
	}

	auto selEntities = m_app->getSelectedEntities();

	ccHObject* entity;
	ccGenericPointCloud* cloud;
	ccPointCloud* pc;
	// onNewSelection already makes sure we have at least one selection
	if (selEntities.size() > 1)
	{
		m_app->dispToConsole("[qTreeIso] Warning: Please only select one entity!", ccMainAppInterface::WRN_CONSOLE_MESSAGE);
		return;
	}
	else
	{
		entity = selEntities[0];
		cloud = ccHObjectCaster::ToGenericPointCloud(selEntities[0]);
		if (cloud && cloud->isA(CC_TYPES::POINT_CLOUD))
		{
			pc = static_cast<ccPointCloud*>(cloud);
		}
		else
		{
			m_app->dispToConsole(QString("Entity [%1] is not of type PointCloud !").arg(selEntities[0]->getName()), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
			return;
		}
	}

	ccHObject* parent = cloud->getParent();
	if (!parent->isGroup())
	{
		m_app->dispToConsole( "[qTreeIso] Warning: Couldn't find group belonging to point cloud !", ccMainAppInterface::WRN_CONSOLE_MESSAGE );
		return;
	}

	unsigned numberOfPoints = pc->size();
	if (numberOfPoints == 0)
	{
		return;
	}

	CCVector3 min, max;
	pc->getBoundingBox(min, max);
	auto minCorner = CCVector3d(min.x, min.y, min.z);

	// COMMENTED SINCE
	// we'd rather not spend that much time searching for a good grid step
	// better to use a rough absolute grid or basic method for rough estimation
	// then find lowest points for those cells and iterate over all the points
	// again to mark all points within a threshold of that height

	//auto octree = pc->getOctree();
	//if (octree == nullptr)
	//{
	//	octree = pc->computeOctree();
	//}

	//// find appropriate cell size for nearest neighbour search
	//double cellSize = std::max(max.x, std::max(max.y, max.z));
	//unsigned char level = 0;
	//constexpr float minCellSize = 0.1f;
	//while (cellSize > minCellSize)
	//{
	//	cellSize /= 2.0f;
	//	++level;
	//}

	//// find grid step by computing avg nearest neighbor distance ?at lowest 2m z slice ?optional
	//CCLib::DgmOctree::NearestNeighboursSearchStruct search_opt;
	//// set query params that only get set once
	//// only query point is updated every time
	//search_opt.level = level;
	//search_opt.minNumberOfNeighbors = 9; //9-18
	//double dNN = 0;
	//unsigned int validPts = 0;
	//const float maxZ = min.z + 2.0f;
	//for (unsigned n = 0; n < numberOfPoints; ++n)
	//{
	//	//for each point
	//	const CCVector3* P = pc->getPoint(n);
	//	if (P->z > maxZ) continue;
	//	++validPts;
	//	search_opt.queryPoint = *P;
	//	octree->getTheCellPosWhichIncludesThePoint(P, search_opt.cellPos, level);
	//	octree->computeCellCenter(search_opt.cellPos, level, search_opt.cellCenter);
	//	search_opt.pointsInNeighbourhood.clear();
	//	search_opt.alreadyVisitedNeighbourhoodSize = 0;

	//	unsigned neighborCount = octree->findNearestNeighborsStartingFromCell(search_opt, false);

	//	double avg_dist_sq = 0;
	//	for (unsigned i = 0; i < neighborCount; ++i)
	//	{
	//		avg_dist_sq += search_opt.pointsInNeighbourhood[i].squareDistd;
	//	}
	//	
	//	dNN += avg_dist_sq / static_cast<double>(neighborCount);
	//}
	//dNN = sqrt(dNN / static_cast<double>(validPts));


	if (!pc->resetVisibilityArray()) // resets visiblity array or allocates it if it wasn't already
	{
		ccLog::Error(QString("[Cloud %1] Visibility table could not be instantiated!").arg(pc->getName()));
		return;
	}
	ccGenericPointCloud::VisibilityTableType& visibilityArray = pc->getTheVisibilityArray();

	// use average distance to nearest neighbour in bottom 2m z-slice as grid-step
	//double gridStep = dNN;
	constexpr double gridStep = 0.1f;
	// changed + 0.5f to + 1.5f since i was running into out of bounds issues
	unsigned int width = static_cast<unsigned int>((max.x - min.x) / gridStep + 1.5f);
	unsigned int height = static_cast<unsigned int>((max.y - min.y) / gridStep + 1.5f);
	const float heightRange = max.z - min.z;
	m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] %1w x %2h = %3 cells").arg(width).arg(height).arg(width*height), ccMainAppInterface::STD_CONSOLE_MESSAGE);
	std::vector<float> lowestHeights;
	try
	{
		lowestHeights.resize(width * height); // CARE! dont use reserve here std::fill won't work as expected etc.

	}
	catch (const std::bad_alloc&)
	{
		return;
	}
	std::fill(lowestHeights.begin(), lowestHeights.end(), std::numeric_limits<float>::max());

	// from ccRasterGrid::fillWith
	// first pass to find lowest heights in grid cell
	constexpr float INCLINE_EPSILON = 0.05f;
	for (unsigned n = 0; n < numberOfPoints; ++n)
	{
		//for each point
		const CCVector3* P = cloud->getPoint(n);

		//project it inside the grid
		CCVector3d relativePos = CCVector3d::fromArray(P->u) - minCorner;
		int x = static_cast<int>((relativePos.u[0] / gridStep + 0.5));
		int y = static_cast<int>((relativePos.u[1] / gridStep + 0.5));

		//we skip points that fall outside of the grid!
		if (x < 0 || x >= static_cast<int>(width)
			|| y < 0 || y >= static_cast<int>(height))
		{
			continue;
		}
		assert(x >= 0 && y >= 0);

		unsigned idx = y * width + x;
		float* cell = &lowestHeights[idx];
		//if (std::abs(P->x - -7.264612) < 0.001f && std::abs(P->y - -18.490040) < 0.001f && std::abs(P->z - 0.044942) < 0.001f)
		//if (std::abs(P->x - -5.308474) < 0.001f && std::abs(P->y - -20.401810) < 0.001f && std::abs(P->z - 1.131417) < 0.001f)
		//{
		//	m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] cell(y%2,x%3) %1").arg(*cell).arg(y).arg(x), ccMainAppInterface::STD_CONSOLE_MESSAGE);
		//}
		//if (y == 82 && x == 162)
		//{
		//	m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] np cell(y%2,x%3) %1").arg(*cell).arg(y).arg(x), ccMainAppInterface::STD_CONSOLE_MESSAGE);
		//}
		if (P->z < *cell)
		{
			m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] smaller z"), ccMainAppInterface::STD_CONSOLE_MESSAGE);
			*cell = P->z;
//#define INCLINE_CMP 1
#if INCLINE_CMP
			// normalize z so we can compare it better
			float pNormZ = relativePos.z / heightRange + INCLINE_EPSILON;
			// de/increase cmp value by INCLINE_EPSILON so we dont miss jumps that are small
			// when measured in an absolute way but huge when measure relatively
			// e.g. from z 0.002 to 0.02
			// TODO: tweak epsilons + this compare factor
			//float cmp = P->z >= 0 ? P->z * 0.5f - INCLINE_EPSILON : P->z * 1.5f + INCLINE_EPSILON;  // use factor+1 when neg z
			float up, down, left, right;
			up = down = left = right = std::numeric_limits<float>::max();
			up = ((y - 1) >= 0) ? (lowestHeights[idx - width] - min.z) / heightRange : up;
			down = ((y + 1) < height) ? (lowestHeights[idx + width] - min.z) / heightRange : down;
			left = ((x - 1) >= 0) ? (lowestHeights[idx - 1] - min.z) / heightRange : left;
			right = ((x + 1) < width) ? (lowestHeights[idx + 1] - min.z) / heightRange : right;

			// test if z of testing point is too much of an increase compared to its neighbours
			if (!(pNormZ > up || pNormZ > down || pNormZ > left || pNormZ > right))
			{
				*cell = P->z;
			}
			else
			{
				//m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] Skipped Z %1 pNormZ+epsilon %2 up %3/%4 down %5/%6 left %7/%8 right %9/%10").arg(P->z).arg(pNormZ).arg(up).arg(up * heightRange).arg(down).arg(down * heightRange).arg(left).arg(left * heightRange).arg(right).arg(right * heightRange), ccMainAppInterface::STD_CONSOLE_MESSAGE);

			}
#endif
		}
	}

//#define MED_CMP 1
#if MED_CMP
	float median = findMedian(lowestHeights);
	float lim = (median - min.z) / heightRange + 0.05f;
	m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] median height: %1").arg(median), ccMainAppInterface::STD_CONSOLE_MESSAGE);
#endif

//#define AVG_NEIGHBOURS 1
#if AVG_NEIGHBOURS
	// average cells with NWSE neighbours so we smooth steep slopes
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			unsigned idx = y * width + x;
			unsigned nneighbours = 0;
			double avg = 0;
			if (lowestHeights[idx] != std::numeric_limits<float>::max())
			{
				avg += lowestHeights[idx];
				++nneighbours;
			}
			// TODO check if unset
			// y/x unsigned cast to in so we can go <0
			float val;
			if (((int)y - 1) >= 0)
			{
				val = lowestHeights[idx - width];
				if (val != std::numeric_limits<float>::max())
				{
					avg += val;
					++nneighbours;
				}
			}
			if ((y + 1) < height)
			{
				val = lowestHeights[idx + width];
				if (val != std::numeric_limits<float>::max())
				{
					avg += val;
					++nneighbours;
				}
			}
			if (((int)x - 1) >= 0)
			{
				val = lowestHeights[idx - 1];
				if (val != std::numeric_limits<float>::max())
				{
					avg += val;
					++nneighbours;
				}
			}
			if ((x + 1) < width)
			{
				val = lowestHeights[idx + 1];
				if (val != std::numeric_limits<float>::max())
				{
					avg += val;
					++nneighbours;
				}
			}

			if (nneighbours)
			{
				avg /= static_cast<double>(nneighbours);
				//m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] cell %1 avg %2").arg(lowestHeights[idx]).arg(avg), ccMainAppInterface::STD_CONSOLE_MESSAGE);
				lowestHeights[idx] = static_cast<float>(avg);
			}
		}
	}
#endif

	m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] minCorner: x %1 y %2 z %3").arg(minCorner.x).arg(minCorner.y).arg(minCorner.z), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
	constexpr float GROUND_PT_EPSILON = 0.03f;
	// SECOND pass to mark all points for extraction that are within a limit of the lowest height
	for (unsigned n = 0; n < numberOfPoints; ++n)
	{
		//for each point
		const CCVector3* P = cloud->getPoint(n);

		//project it inside the grid
		CCVector3d relativePos = CCVector3d::fromArray(P->u) - minCorner;
		int x = static_cast<int>((relativePos.u[0] / gridStep + 0.5));
		int y = static_cast<int>((relativePos.u[1] / gridStep + 0.5));

		//we skip points that fall outside of the grid!
		if (x < 0 || x >= static_cast<int>(width)
			|| y < 0 || y >= static_cast<int>(height))
		{
			m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] OUT OF BOUNDS! y %1 x %2").arg(y).arg(x), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
			m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] on point: x %1 y %2 z %3").arg(P->x).arg(P->y).arg(P->z), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
			continue;
		}
		assert(x >= 0 && y >= 0);

		unsigned idx = y * width + x;
		float cell = lowestHeights[idx];
		// decided to rather do this check here so the values we take from neighbouring fields don't
		// propagate and we miss areas following a steeper incline
		// check if cell value unset due to not passing neighbour checks
#if INCLINE_CMP
		if (cell == std::numeric_limits<float>::max())
		{
			//m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] Found unset on y %1 x %2").arg(y).arg(x), ccMainAppInterface::STD_CONSOLE_MESSAGE);

			// find neighbouring cells that are set
			// float up, down, left, right = std::numeric_limits<float>::max(); => only right is initialized!!!
			constexpr float fmax = std::numeric_limits<float>::max();
			float up, down, left, right;
			up = down = left = right = fmax;
			for(int i = 1; (up != fmax && down != fmax && left != fmax && right != fmax) && (i < static_cast<int>(std::min(width, height) * 0.3f)); ++i)
			{
				if(up == fmax)
					up = ((y - i) >= 0) ? lowestHeights[idx - i*width] : up;
				if(down == fmax)
					down = ((y + i) < height) ? lowestHeights[idx + i*width] : down;
				if(left == fmax)
					left = ((x - i) >= 0) ? lowestHeights[idx - i] : left;
				if(right == fmax)
					right = ((x + i) < width) ? lowestHeights[idx + i] : right;
			}
			// use min of neighbours otherwise this cell might remain at float max
			// update local AND cell in grid
			lowestHeights[idx] = cell = std::min(up, std::min(down, std::min(left, right)));
		}
#endif
#if MED_CMP
		if (((cell - min.z) / heightRange) > lim)
		{
			float up, down, left, right;
			up = down = left = right = 1.5f;//std::numeric_limits<float>::max();
			for(int i = 1; (up > lim && down > lim && left > lim && right > lim) && (i < static_cast<int>(std::min(width, height) * 0.3f)); ++i)
			{
				if (up > lim && ((y - i) >= 0))
				{
					up = lowestHeights[idx - i * width];
					up = ((up - min.z) / heightRange) <= lim ? up : 1.5f;
				}
				if (down > lim && ((y + i) < height))
				{
					down = lowestHeights[idx + i * width];
					down = ((down - min.z) / heightRange) <= lim ? down : 1.5f;
				}
				if (left > lim && ((x - i) >= 0))
				{
					left = lowestHeights[idx - i];
					left = ((left - min.z) / heightRange) <= lim ? left : 1.5f;
				}
				if (right > lim && ((x + i) < width))
				{
					right = lowestHeights[idx + i];
					right = ((right - min.z) / heightRange) <= lim ? right : 1.5f;
				}
			}
			m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] cell %1 too high").arg(cell), ccMainAppInterface::STD_CONSOLE_MESSAGE);
			// search from neighbour outwards till we find heights that dont exceed our height limit
			// update local AND cell in grid
			lowestHeights[idx] = cell = std::min(up, std::min(down, std::min(left, right)));
			m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] replacing with %1").arg(cell), ccMainAppInterface::STD_CONSOLE_MESSAGE);
		}
#endif
		if (P->z >= (cell + GROUND_PT_EPSILON))
		{
			// hide non ground pts
			visibilityArray[n] = POINT_HIDDEN;
		}
	}
	//we temporarily detach the entity, as it may undergo
	//"severe" modifications (octree deletion, etc.) --> see ccPointCloud::createNewCloudFromVisibilitySelection
	// taken from MainWindow::deactivateSegmentationMode
	ccMainAppInterface::ccHObjectContext objContext = m_app->removeObjectTemporarilyFromDBTree(entity);

	auto extractedPoints = cloud->createNewCloudFromVisibilitySelection(true);
	if (!extractedPoints)
	{
		m_app->dispToConsole(QString("[qTreeIso:RemoveLowestPoints] Failed to create extracted cloud ! (not enough memory)"), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
		return;
	}
	else
	{
		//'shift on load' information
		extractedPoints->setGlobalShift(pc->getGlobalShift());
		extractedPoints->setGlobalScale(pc->getGlobalScale());

		extractedPoints->setVisible(true);
		extractedPoints->setName(QString("%1_GROUND").arg(cloud->getName()));

		//we add new CC to parent
		parent->addChild(extractedPoints);
		m_app->addToDB(extractedPoints);

		m_app->setSelectedInDB(extractedPoints, true);
	}

	m_app->putObjectBackIntoDBTree(entity, objContext);

	entity->setEnabled(false);

	//redraw
	m_app->updateUI();
	m_app->getActiveGLWindow()->redraw();
}

void qTreeIso::doBHDSlice()
{
	m_onPointPicked = DO_BHD_SLICE;
	startPicking();
}

void qTreeIso::pointPickedBHDSlice(ccHObject* entity, unsigned itemIdx, int x, int y, const CCVector3& P)
{
	if (!entity) //null pick
	{
		return;
	}

	// make sure we picked a point cloud?
	if (!entity->isKindOf(CC_TYPES::POINT_CLOUD))
	{
		m_app->dispToConsole( "[qTreeIso] Warning: Please select a point that's part of a point cloud !", ccMainAppInterface::WRN_CONSOLE_MESSAGE );
		return;
	}
	//get point cloud
	// ccGenericPointCloud* genCloud = ccHObjectCaster::ToGenericPointCloud(entity);
	ccPointCloud* cloud = static_cast<ccPointCloud*>(entity); //cast to point cloud
	if (!cloud)
	{
		ccLog::Warning("[Item picking] Shit's fubar (Picked point is not in pickable entities DB?)!");
		return;
	}

	// push minZ 1.25m up from !picked! starting point and set maxZ to minZ + 10cm so we get 5cm above and under
	// the 1.3m needed for BHD measurement
	ccBBox cropBox(entity->getOwnBB());
	cropBox.minCorner().z = P.z + 1.25f;
	cropBox.maxCorner().z = cropBox.minCorner().z + 0.1f;

	// from ccCropTool.cpp since i somehow can't include qCC/ccCropTool.h
	// ccHObject* croppedEnt = ccCropTool::Crop(entity, cropBox, true);
	CCLib::ReferenceCloud* selection = cloud->crop(cropBox, true);
	if (!selection)
	{
		//process failed!
		m_app->dispToConsole(QString("[Crop] Failed to crop cloud '%1'!").arg(cloud->getName()), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
		return;
	}

	if (selection->size() == 0)
	{
		//no points fall inside selection!
		m_app->dispToConsole(QString("[Crop] No point of the cloud '%1' falls inside the input box!").arg(cloud->getName()), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
		delete selection;
		return;
	}

	//crop
	ccPointCloud* croppedEnt = cloud->partialClone(selection);
	delete selection;
	selection = 0;
	//if (croppedEnt) // technically not necessary since we already checked selection but this is a remnat of using ccCropTool
	croppedEnt->setName(entity->getName() + QString(".BHD-slice"));
	croppedEnt->setDisplay(entity->getDisplay());
	croppedEnt->prepareDisplayForRefresh();
	if (entity->getParent())
		entity->getParent()->addChild(croppedEnt, 24/*default*/, entity->getIndex()); // insert bhd slice above origin cloud
	entity->setEnabled(false);
	m_app->addToDB(croppedEnt);
	//select output entity
	m_app->setSelectedInDB(croppedEnt, true);

	stopPicking();
	m_app->refreshAll();
	m_app->updateUI();
}

// definition of default parameters in header file
void qTreeIso::splitCloudBy(bool splitByColor, bool selectClouds, bool randomColors)
{	
	if ( m_app == nullptr )
	{
		// m_app should have already been initialized by CC when plugin is loaded
		Q_ASSERT( false );
		
		return;
	}

	/*** HERE STARTS THE ACTION ***/

	// Put your code here
	// --> you may want to start by asking for parameters (with a custom dialog, etc.)
	
	// Display an error message in the console AND pop-up an error box
	// m_app->dispToConsole( "Example plugin shouldn't be used - it doesn't do anything!", ccMainAppInterface::ERR_CONSOLE_MESSAGE );

	auto selEntities = m_app->getSelectedEntities();
	
	ccHObject* entity;
	ccGenericPointCloud* cloud;
	ccPointCloud* pc;
	// onNewSelection already makes sure we have at least one selection
	if (selEntities.size() > 1)
	{
		m_app->dispToConsole( "[qTreeIso] Warning: Please only select one entity!", ccMainAppInterface::WRN_CONSOLE_MESSAGE );
		return;
	}
	else
	{
		entity = selEntities[0];
		cloud = ccHObjectCaster::ToGenericPointCloud(selEntities[0]);
		if (cloud && cloud->isA(CC_TYPES::POINT_CLOUD))
		{
			pc = static_cast<ccPointCloud*>(cloud);
			//la methode est activee sur le champ scalaire affiche
			if (!splitByColor && !pc->getCurrentDisplayedScalarField())
			{
				m_app->dispToConsole(QString("Entity [%1] has no active scalar field !").arg(selEntities[0]->getName()), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
				return;
			}
			else if (splitByColor && !pc->hasColors())
			{
				m_app->dispToConsole(QString("Entity [%1] has no colors !").arg(selEntities[0]->getName()), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
				return;
			}
		}
		else
		{
			m_app->dispToConsole(QString("Entity [%1] is not of type PointCloud !").arg(selEntities[0]->getName()), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
			return;
		}
	}

	unsigned numberOfPoints = pc->size();
	if (numberOfPoints == 0)
	{
		return;
	}

	// maps color values to clouds
	std::map<int, CCLib::ReferenceCloud*> scalarCloudMap;
	for (unsigned i = 0; i < numberOfPoints; ++i)
	{
		// ScalarType slabel = pc->getPointScalarValue(i);
		int scalarVal;
		if (splitByColor)
		{
			auto rgb = pc->getPointColor(i);
			// or channel values together to get single unique int per color
			scalarVal = (rgb.r << 16) | (rgb.g << 8) | (rgb.b << 0);
		}
		else
		{
			scalarVal = static_cast<int>(pc->getPointScalarValue(i));
		}

		// check if we already have a refcloud in the map for this scalar
		if (!scalarCloudMap.count(scalarVal))
		{
			try
			{
				scalarCloudMap.insert(std::pair<int, CCLib::ReferenceCloud*>(scalarVal, new CCLib::ReferenceCloud(pc)));
			}
			catch (const std::bad_alloc&)
			{
				// not enough memory
				// std::map does not manage the memory pointed to by the refcloud ptrs we store in there
				// need to delete them manually
				for (auto it = scalarCloudMap.begin(); it != scalarCloudMap.end(); ++it)
					delete it->second;
				scalarCloudMap.clear();
				return;
			}
		}

		// get cloud corresponding to scalar from map
		auto refCloud = scalarCloudMap[scalarVal];
		if (!refCloud->addPointIndex(i))
		{
			// out of memory
			for (auto it = scalarCloudMap.begin(); it != scalarCloudMap.end(); ++it)
				delete it->second;
			scalarCloudMap.clear();
			return;
		}
	}

	// print debugging for checking key values
	//for (auto it=scalarCloudMap.begin(); it != scalarCloudMap.end(); ++it)
	//	m_app->dispToConsole( QString("[qTreeIso] RGB val: %1!").arg(it->first), ccMainAppInterface::STD_CONSOLE_MESSAGE );


	//we create "real" point clouds for all input components
	{
		// ccPointCloud* pc = cloud->isA(CC_TYPES::POINT_CLOUD) ? static_cast<ccPointCloud*>(cloud) : 0;

		//we create a new group to store all CCs
		ccHObject* ccGroup = new ccHObject(cloud->getName() + QString(" [Splits]"));

		//for each component
		for (auto it=scalarCloudMap.begin(); it != scalarCloudMap.end(); ++it)
		{
			//CCLib::ReferenceCloud* compIndexes = _sortedIndexes ? components[_sortedIndexes->at(i).index] : components[i];

			//if it has enough points
			//if (compIndexes->size() >= minPointsPerComponent)

			//we create a new entity
			// ccPointCloud* compCloud = (pc ? pc->partialClone(compIndexes) : ccPointCloud::From(compIndexes));
			ccPointCloud* splitCloud = pc->partialClone(it->second);
			if (splitCloud)
			{
				//shall we colorize it with random color?
				if (randomColors)
				{
					ccColor::Rgb col = ccColor::Generator::Random();
					splitCloud->setRGBColor(col);
				}
				if (splitByColor || randomColors)
				{
					splitCloud->showColors(true);
					splitCloud->showSF(false);
				}

				//'shift on load' information
				if (pc)
				{
					splitCloud->setGlobalShift(pc->getGlobalShift());
					splitCloud->setGlobalScale(pc->getGlobalScale());
				}
				splitCloud->setVisible(true);
				splitCloud->setName(QString("Split#%1").arg(ccGroup->getChildrenNumber()));

				//we add new CC to group
				ccGroup->addChild(splitCloud);

				if (selectClouds)
					m_app->setSelectedInDB(splitCloud, true);
			}
			else
			{
				m_app->dispToConsole(QString("[qTreeIso:Split] Failed to create split cloud %1! (not enough memory)").arg(ccGroup->getChildrenNumber()+1), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
			}

			delete it->second;
			// compIndexes = nullptr;
		}

		scalarCloudMap.clear();

		if (ccGroup->getChildrenNumber() == 0)
		{
			m_app->dispToConsole("No split cloud was created! Check the minimum size...", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
			delete ccGroup;
			ccGroup = nullptr;
		}
		else
		{
			ccGroup->setDisplay(cloud->getDisplay());
			m_app->addToDB(ccGroup);

			m_app->dispToConsole(QString("[qTreeIso:Split] %1 split cloud(s) were created from cloud '%2'").arg(ccGroup->getChildrenNumber()).arg(cloud->getName()));
		}

		cloud->prepareDisplayForRefresh();

		//auto-hide original cloud
		if (ccGroup)
		{
			cloud->setEnabled(false);
			m_app->dispToConsole("[qTreeIso:Split] Original cloud has been automatically hidden", ccMainAppInterface::WRN_CONSOLE_MESSAGE);
		}
	}

	//ccHObject::Container results;
	//{
	//	//CCLib::ScalarField* sf = pc->getCurrentDisplayedScalarField();
	//	//assert(sf);

	//	//we set as output (OUT) the currently displayed scalar field
	//	int outSfIdx = pc->getCurrentDisplayedScalarFieldIndex();
	//	assert(outSfIdx >= 0);
	//	pc->setCurrentOutScalarField(outSfIdx);
	//	//pc->setCurrentScalarField(outSfIdx);

	//	ccHObject* resultInside = nullptr;
	//	ccHObject* resultOutside = nullptr;
	//	//if (entity->isKindOf(CC_TYPES::MESH))
	//	//{
	//	//	pc->hidePointsByScalarValue(minVal, maxVal);
	//	//	if (entity->isA(CC_TYPES::MESH)/*|| entity->isKindOf(CC_TYPES::PRIMITIVE)*/) //TODO
	//	//		resultInside = ccHObjectCaster::ToMesh(entity)->createNewMeshFromSelection(false);
	//	//	else if (entity->isA(CC_TYPES::SUB_MESH))
	//	//		resultInside = ccHObjectCaster::ToSubMesh(entity)->createNewSubMeshFromSelection(false);

	//	//	if (mode == ccFilterByValueDlg::SPLIT)
	//	//	{
	//	//		pc->invertVisibilityArray();
	//	//		if (entity->isA(CC_TYPES::MESH)/*|| entity->isKindOf(CC_TYPES::PRIMITIVE)*/) //TODO
	//	//			resultOutside = ccHObjectCaster::ToMesh(entity)->createNewMeshFromSelection(false);
	//	//		else if (entity->isA(CC_TYPES::SUB_MESH))
	//	//			resultOutside = ccHObjectCaster::ToSubMesh(entity)->createNewSubMeshFromSelection(false);
	//	//	}

	//	//	pc->unallocateVisibilityArray();
	//	//}
	//	//if (entity->isKindOf(CC_TYPES::POINT_CLOUD))
	//	//{
	//	//	//pc->hidePointsByScalarValue(minVal,maxVal);
	//	//	//result = ccHObjectCaster::ToGenericPointCloud(entity)->hidePointsByScalarValue(false);
	//	//	//pc->unallocateVisibilityArray();

	//	//	//shortcut, as we know here that the point cloud is a "ccPointCloud"
	//	//	resultInside = pc->filterPointsByScalarValue(minVal, maxVal, false);

	//	//	if (mode == ccFilterByValueDlg::SPLIT)
	//	//	{
	//	//		resultOutside = pc->filterPointsByScalarValue(minVal, maxVal, true);
	//	//	}
	//	//}

	//	if (resultInside)
	//	{
	//		entity->setEnabled(false);
	//		resultInside->setDisplay(entity->getDisplay());
	//		resultInside->prepareDisplayForRefresh();
	//		m_app->addToDB(resultInside);

	//		results.push_back(resultInside);
	//	}
	//	if (resultOutside)
	//	{
	//		entity->setEnabled(false);
	//		resultOutside->setDisplay(entity->getDisplay());
	//		resultOutside->prepareDisplayForRefresh();
	//		resultOutside->setName(resultOutside->getName() + ".outside");
	//		m_app->addToDB(resultOutside);

	//		results.push_back(resultOutside);
	//	}
	//	//*/
	//}

	//if (!results.empty())
	//{
	//	ccConsole::Warning("Previously selected entities (sources) have been hidden!");
	//	if (m_ccRoot)
	//	{
	//		m_ccRoot->selectEntities(results);
	//	}
	//}

	//refreshAll();

	/*** HERE ENDS THE ACTION ***/
}
