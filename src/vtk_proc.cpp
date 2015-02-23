#include "vtk_proc.h"

Vtk_proc::Vtk_proc(FE_Storage_Interface *st, string _fileName) : Post_proc(st)
{
	name = "Vtk_processor";
	file_name = _fileName;
	comp_codes.push_back(E_1);
	comp_codes.push_back(E_2);
	comp_codes.push_back(E_3);
	comp_codes.push_back(E_VOL);
	comp_codes.push_back(S_1);
	comp_codes.push_back(S_2);
	comp_codes.push_back(S_3);
	comp_codes.push_back(S_P);
}

Vtk_proc::~Vtk_proc()
{
}

void Vtk_proc::pre(uint16 qLoadstep)
{
	string cur_fn = file_name +  IntToStr(0) + ".vtk";
	ofstream file(cur_fn, ios::trunc);
	write_header(file);
	write_geometry(file, false);
	write_point_data(file, true);
	write_cell_data(file,true);
	file.close();
}

void Vtk_proc::process (uint16 curLoadstep, uint16 qLoadstep)
{
	string cur_fn = file_name +  IntToStr(curLoadstep) + ".vtk";
	ofstream file(cur_fn, ios::trunc);
	write_header(file);
	write_geometry(file, true);
	write_point_data(file, false);
	write_cell_data(file,false);
	file.close();
}

void Vtk_proc::post (uint16 curLoadstep, uint16 qLoadstep)
{
}

void Vtk_proc::write_header (ofstream &file)
{
	file << "# vtk DataFile Version 2.0" << endl;
	file << "This file is generated by NLA program" << endl << "ASCII" << endl;
	file << "DATASET UNSTRUCTURED_GRID" << endl;
}

void Vtk_proc::write_geometry(ofstream &file, bool def)
{
	Vec<3> xi;
	file << "POINTS " << storage->getNumNode() << " float" << endl;
	for (uint32 i=1; i <= storage->getNumNode(); i++)
	{
		storage->get_node_pos(i, xi.ptr(), def);
		file << xi << endl;
	}
	/*
	CELLS en en*9
	4 i j k l

	CELL_TYPES en
	11
	11

	CELL_DATA en
	TENSORS Couchy_stresses float
	t11 t12 t13
	..  ..  ..
	t31 t32 t33
	*/

	file << "CELLS " << storage->getNumElement() << " " <<  storage->getNumElement()*(Element::n_nodes()+1) << endl;
	for (uint32 i=1; i <= storage->getNumElement(); i++)
	{
    uint16 nodesNum = storage->getElement(i).n_nodes();
		//uint16 order[] = {0,1,3,2,4,5,7,6};
		file << nodesNum;
		for (uint16 j=0; j < nodesNum; j++) 
			file << " " << storage->getElement(i).node_num(j)-1;
		file << endl;
	}
	file << "CELL_TYPES " << storage->getNumElement() << endl;
	for (uint32 i=1; i <= storage->getNumElement(); i++) {
    uint16 nodesNum = storage->getElement(i).n_nodes();
    if (nodesNum == 4) {
      file << "9" << endl; //VTK_QUAD, see VTK file formats
    } else if (nodesNum == 8) {
      file << "12" << endl; //VTK_HEXAHEDRON
    } else {
      error("I don't now what type of elements here it is (el_num = %d)", i);
    }
  }
}

void Vtk_proc::write_point_data(ofstream &file, bool zero)
{
	double val, tmp;
	file << "POINT_DATA " << storage->getNumNode() << endl;
	//vector<string> ux_labels;
	char* ux_labels[] = {"ux", "uy", "uz"}; //NEW: CHECK
	//ux_labels.push_back("ux");
	//ux_labels.push_back("uy");
	//ux_labels.push_back("uz");
	for (uint16 i=0; i < Element::n_dim(); i++)
	{
		file << "SCALARS " << ux_labels[i] << " float 1" << endl;
		file << "LOOKUP_TABLE default"<< endl;
		for (uint32 j=1; j <= storage->getNumNode(); j++)
			if (zero)
				file << 0.0 << endl;
			else
				file << storage->get_qi_n(j, i) << endl;
	}
	//usym
	file << "SCALARS usym float 1" << endl;
	file << "LOOKUP_TABLE default"<< endl;
	for (uint32 j=1; j <= storage->getNumNode(); j++)
	{
		val = 0.0f;
		if (!zero)
		{
			for (uint16 i=0; i < Element::n_dim(); i++)
			{
				tmp = storage->get_qi_n(j, i);
				val += tmp*tmp;
			}
			val = sqrt(val);
		}
		file << val << endl;
	}
	//DEBUG
//	for (uint16 lab_i=0; lab_i < comp_codes.size(); lab_i++)
//	{
//		file << "SCALARS " << el_component_labels[lab_i] << " float 1" << endl;
//		file << "LOOKUP_TABLE default"<< endl;
//		if (zero)
//			for (uint32 i=0; i < storage->getNumNode(); i++)
//				file << 0.0f << endl;
//		else
//		{
//			write_pointed_el_component3(file, comp_codes[lab_i]);
//		}
//	}
}
////интреполяция по трем точкам интегрирования
//void Vtk_proc::write_pointed_el_component(ofstream &file, uint16 code)
//{
//	vector<double> nodes;
//	vector<uint32> count;
//	nodes.assign(storage->nn,0);
//	count.assign(storage->nn,0);
//	uint16 gps[3] = {2,4,8}; //точки интегрирования, по которым происходит интерполяция
//	double x[3], y[3], p[3], det, a, b , c;
//	for (uint32 i=0; i < storage->en; i++)
//	{
//		//if (i == 176) continue;
//		for (uint16 j=0; j < 3; j++)
//		{
//			x[j] =storage->el_form[i]->getComponent(gps[j],POS_X,i,storage); 
//			y[j] =storage->el_form[i]->getComponent(gps[j],POS_Y,i,storage); 
//			p[j] =storage->el_form[i]->getComponent(gps[j],code,i,storage); 
//		}
//		det = x[0]*y[1]-x[1]*y[0]-x[0]*y[2]+x[2]*y[0]+x[1]*y[2]-x[2]*y[1];
//		a = 1/det*(p[0]*(y[1]-y[2])-p[1]*(y[0]-y[2])+p[2]*(y[0]-y[1]));
//		b = 1/det*(-p[0]*(x[1]-x[2])+p[1]*(x[0]-x[2])-p[2]*(x[0]-x[1]));
//		c = 1/det*(p[0]*(x[1]*y[2]-x[2]*y[1])-p[1]*(x[0]*y[2]-x[2]*y[0]) + p[2]*(x[0]*y[1]-x[1]*y[0]));
//		for (uint16 j=0; j < 4; j++)
//		{
//			double x = storage->nodes[storage->elements[i].nodes[j]-1].coord[0];
//			double y = storage->nodes[storage->elements[i].nodes[j]-1].coord[1];
//			nodes[storage->elements[i].nodes[j]-1] += a*x+b*y+c;
//			count[storage->elements[i].nodes[j]-1] += 1;
//		}
//	}
//	for (uint32 i=0; i < nodes.size(); i++)
//	{
//		file << nodes[i]/count[i] << endl;
//	}
//}
//
//// по ближайшим точкам интегрирования к узлам
//void Vtk_proc::write_pointed_el_component2(ofstream &file, uint16 code)
//{
//	vector<double> nodes;
//	vector<uint32> count;
//	nodes.assign(storage->nn,0);
//	count.assign(storage->nn,0);
//	uint16 gps[4] = {0,2,8,6}; //точки интегрирования, по которым происходит интерполяция
//	double x[3], y[3], p[3], det, a, b , c;
//	for (uint32 i=0; i < storage->en; i++)
//	{
//		for (uint16 j=0; j < 4; j++)
//		{
//			nodes[storage->elements[i].nodes[j]-1] += storage->el_form[i]->getComponent(gps[j],code,i,storage);
//			count[storage->elements[i].nodes[j]-1] += 1;
//		}
//	}
//	for (uint32 i=0; i < nodes.size(); i++)
//	{
//		file << nodes[i]/count[i] << endl;
//	}
//}

//по центральной точке интегрирования
void Vtk_proc::write_pointed_el_component3(ofstream &file, el_component code)
{
	vector<double> nodes; //TODO: do it
	vector<uint32> count;
	nodes.assign(storage->getNumNode(),0);
	count.assign(storage->getNumNode(),0);
	for (uint32 i=1; i <= storage->getNumElement(); i++)
	{
		for (uint16 j=0; j < Element::n_nodes(); j++)
		{
			nodes[storage->getElement(i).node_num(j)-1] += storage->getElement(i).getComponent(Element::get_central_gp(),code,i,storage);
			count[storage->getElement(i).node_num(j)-1] += 1;
		}
	}
	for (uint32 i=0; i < nodes.size(); i++)
	{
		file << nodes[i]/count[i] << endl;
	}
}


void Vtk_proc::write_cell_data(ofstream &file, bool zero)
{
	file << "CELL_DATA " << storage->getNumElement() << endl;
	el_tensor type = TENS_COUCHY;
	Mat<3,3> tens;
	vector<double> data[6];
	char* names[] = {"SR","ST", "SZ" ,"SRT", "STZ", "SRZ"};

	//file << "TENSORS " << el_tensor_labels[type] << " float" << endl;
	for (uint32 i=1; i <= storage->getNumElement(); i++)
	{
		if (!zero)
			tens = storage->getElement(i).getTensor(Element::get_central_gp(), type, i, storage);
		data[0].push_back(tens[0][0]); //xx
		data[1].push_back(tens[1][1]); //yy
		data[2].push_back(tens[2][2]); //zz
		data[3].push_back(tens[0][1]); //xy
		data[4].push_back(tens[1][2]); //yz
		data[5].push_back(tens[0][2]); //xz
	}

	for (uint16 ii=0;ii<6;ii++)
	{
		file << "SCALARS " << names[ii] << " float 1" << endl;
		file << "LOOKUP_TABLE default"<< endl;
		for (uint32 i=1; i <= storage->getNumElement(); i++) 
		{
			file << data[ii][i-1] << endl;
		}
	}
	
	for (uint16 lab_i=0; lab_i < comp_codes.size(); lab_i++)
	{
		file << "SCALARS " << el_component_labels[comp_codes[lab_i]] << " float 1" << endl;
		file << "LOOKUP_TABLE default"<< endl;
		for (uint32 i=1; i <= storage->getNumElement(); i++)
			//for (uint16 j=0; j < Element::n_face(); j++)
				if (zero)
					file << 0.0f << endl;
				else
					file << storage->getElement(i).getComponent(Element::get_central_gp(), comp_codes[lab_i], i, storage) << endl;
	}
	
}
