#include "board.hpp"

typedef pair<string, shared_ptr<Layer> > layer_t;

Board::Board()
{
        margin = 0.0;
	dpi = 1000;
}

double
Board::get_width()
{
	return layers.begin()->second->surface->get_width_in();
}

double
Board::get_height()
{
	return layers.begin()->second->surface->get_height_in();
}


void
Board::prepareLayer( string layername, shared_ptr<LayerImporter> importer, shared_ptr<RoutingMill> manufacturer, bool mirror )
{
        prepared_layers.insert( std::make_pair( layername, make_tuple(importer, manufacturer, mirror) ) );
}

void
Board::createLayers()
{
        // start calculating the minimal board size

	min_x = 10000.0;          // not pretty, but ok for now
	max_x = -10000.0;
	min_y = 10000.0;
	max_y = -10000.0;

        // calculate room needed by the PCB traces
        for( map< string, prep_t >::iterator it = prepared_layers.begin(); it != prepared_layers.end(); it++ ) {
                shared_ptr<LayerImporter> importer = it->second.get<0>();
                float t;
                t = importer->get_min_x();
                if(min_x > t) min_x = t;
                t = importer->get_max_x();
                if(max_x < t) max_x = t;
                t = importer->get_min_y();
                if(min_y > t) min_y = t;
                t = importer->get_max_y();
                if(max_y < t) max_y = t;
        }

        // if there's no pcb outline, add the specified margins
        try {
                prepared_layers.at("outline");
        }
	catch( std::logic_error& e ) {
                min_x -= margin;
                max_x += margin;
                min_y -= margin;
                max_y += margin;
        }

        // board size calculated. create layers
        for( map<string, prep_t>::iterator it = prepared_layers.begin(); it != prepared_layers.end(); it++ ) {
		// prepare the surface
                shared_ptr<Surface> surface( new Surface(dpi, min_x, max_x, min_y, max_y) );
                shared_ptr<LayerImporter> importer = it->second.get<0>();
                surface->render(importer);

		shared_ptr<Layer> layer( new Layer(it->first, surface, it->second.get<1>(), it->second.get<2>() ) );
                
		layers.insert( std::make_pair( layer->get_name(), layer ) );
        }

        // DEBUG output
        BOOST_FOREACH( layer_t layer, layers ) {
                layer.second->surface->save_debug_image();
        }
}

vector< shared_ptr<icoords> >
Board::get_toolpath( string layername )
{
	vector< shared_ptr<icoords> > toolpath;

	try {
		return layers[layername]->get_toolpaths();
	} catch ( std::logic_error& e ) {
		std::stringstream msg;
		msg << "class Board: get_toolpath(): layer not available: ";
		msg << layername << std::endl;
		throw std::logic_error( msg.str() );
	}
}

vector< string >
Board::list_layers()
{
        vector<string> layerlist;

        BOOST_FOREACH( layer_t layer, layers ) {
                layerlist.push_back( layer.first );
        }

        return layerlist;
}

shared_ptr<Layer>
Board::get_layer( string layername )
{
	return layers.at(layername);
}