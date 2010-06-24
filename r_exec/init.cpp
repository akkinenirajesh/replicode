#include	"init.h"
#include	"object.h"
#include	"operator.h"
#include	"opcodes.h"

#include	"../r_comp/preprocessor.h"


namespace	r_exec{

	dll_export	uint64	(*Now)();

	dll_export	r_comp::Metadata	Metadata;
	dll_export	r_comp::Image		Seed;

	UNORDERED_MAP<std::string,uint16>	_Opcodes;

	dll_export	r_comp::Compiler		Compiler;
	r_exec_dll	r_comp::Preprocessor	Preprocessor;

	bool	Compile(std::istream	&source_code,std::string	&error,bool	compile_metadata){

		std::ostringstream	preprocessed_code_out;
		if(!r_exec::Preprocessor.process(&source_code,&preprocessed_code_out,error,compile_metadata?&Metadata:NULL))
			return	false;

		std::istringstream	preprocessed_code_in(preprocessed_code_out.str());

		if(!r_exec::Compiler.compile(&preprocessed_code_in,&Seed,&Metadata,error,false)){

			std::streampos	i=preprocessed_code_in.tellg();
			std::cerr.write(preprocessed_code_in.str().c_str(),i);
			std::cerr<<" <- "<<error<<std::endl;
			return	false;
		}

		return	true;
	}

	bool	Compile(const	char	*filename,std::string	&error,bool	compile_metadata){

		std::ifstream	source_code(filename);
		if(!source_code.good()){

			error="unable to load file ";
			error+=filename;
			return	false;
		}

		bool	r=Compile(source_code,error,compile_metadata);
		source_code.close();
		return	r;
	}

	bool	Compile(const	char	*filename,std::string	&error){

		return	Compile(filename,error,false);
	}

	bool	Compile(std::istream	&source_code,std::string	&error){

		return	Compile(source_code,error,false);
	}

	uint16	RetrieveOpcode(const	char	*name){

		return	_Opcodes.find(name)->second;
	}

	bool	Init(const	char	*user_operator_library_path,
				uint64			(*time_base)()){

		UNORDERED_MAP<std::string,r_comp::Class>::iterator it;
		for(it=Metadata.classes.begin();it!=Metadata.classes.end();++it){

			_Opcodes[it->first]=it->second.atom.asOpcode();
			//std::cout<<it->first<<":"<<it->second.atom.asOpcode()<<std::endl;
		}
		for(it=Metadata.sys_classes.begin();it!=Metadata.sys_classes.end();++it){

			_Opcodes[it->first]=it->second.atom.asOpcode();
			//std::cout<<it->first<<":"<<it->second.atom.asOpcode()<<std::endl;
		}

		//	load class Opcodes.
		View::ViewOpcode=_Opcodes.find("view")->second;

		Opcodes::Group=_Opcodes.find("grp")->second;

		Opcodes::PTN=_Opcodes.find("ptn")->second;
		Opcodes::AntiPTN=_Opcodes.find("|ptn")->second;

		Opcodes::IPGM=_Opcodes.find("ipgm")->second;
		Opcodes::PGM=_Opcodes.find("pgm")->second;
		Opcodes::AntiPGM=_Opcodes.find("|pgm")->second;

		Opcodes::IGoal=_Opcodes.find("igol")->second;
		Opcodes::Goal=_Opcodes.find("gol")->second;
		Opcodes::AntiGoal=_Opcodes.find("|gol")->second;

		Opcodes::MkRdx=_Opcodes.find("mk.rdx")->second;
		Opcodes::MkAntiRdx=_Opcodes.find("mk.|rdx")->second;

		Opcodes::MkNew=_Opcodes.find("mk.new")->second;

		Opcodes::MkLowRes=_Opcodes.find("mk.low_res")->second;
		Opcodes::MkLowSln=_Opcodes.find("mk.low_sln")->second;
		Opcodes::MkHighSln=_Opcodes.find("mk.high_sln")->second;
		Opcodes::MkLowAct=_Opcodes.find("mk.low_act")->second;
		Opcodes::MkHighAct=_Opcodes.find("mk.high_act")->second;
		Opcodes::MkSlnChg=_Opcodes.find("mk.sln_chg")->second;
		Opcodes::MkActChg=_Opcodes.find("mk.act_chg")->second;

		//	load executive function Opcodes.
		Opcodes::Inject=_Opcodes.find("_inj")->second;
		Opcodes::Eject=_Opcodes.find("_eje")->second;
		Opcodes::Mod=_Opcodes.find("_eje")->second;
		Opcodes::Set=_Opcodes.find("_eje")->second;
		Opcodes::NewClass=_Opcodes.find("_new_class")->second;
		Opcodes::DelClass=_Opcodes.find("_del_class")->second;
		Opcodes::LDC=_Opcodes.find("_ldc")->second;
		Opcodes::Swap=_Opcodes.find("_swp")->second;
		Opcodes::NewDev=_Opcodes.find("_new_dev")->second;
		Opcodes::DelDev=_Opcodes.find("_del_dev")->second;
		Opcodes::Suspend=_Opcodes.find("_suspend")->second;
		Opcodes::Resume=_Opcodes.find("_resume")->second;
		Opcodes::Stop=_Opcodes.find("_stop")->second;

		//	load std operators.
		uint16	operator_opcode=0;
		Operator::Register(operator_opcode++,now);
		Operator::Register(operator_opcode++,equ);
		Operator::Register(operator_opcode++,neq);
		Operator::Register(operator_opcode++,gtr);
		Operator::Register(operator_opcode++,lsr);
		Operator::Register(operator_opcode++,gte);
		Operator::Register(operator_opcode++,lse);
		Operator::Register(operator_opcode++,add);
		Operator::Register(operator_opcode++,sub);
		Operator::Register(operator_opcode++,mul);
		Operator::Register(operator_opcode++,div);
		Operator::Register(operator_opcode++,dis);
		Operator::Register(operator_opcode++,ln);
		Operator::Register(operator_opcode++,exp);
		Operator::Register(operator_opcode++,log);
		Operator::Register(operator_opcode++,e10);
		Operator::Register(operator_opcode++,syn);
		Operator::Register(operator_opcode++,ins);
		Operator::Register(operator_opcode++,at);
		Operator::Register(operator_opcode++,red);
		Operator::Register(operator_opcode++,com);
		Operator::Register(operator_opcode++,spl);
		Operator::Register(operator_opcode++,mrg);
		Operator::Register(operator_opcode++,ptc);
		Operator::Register(operator_opcode++,fvw);

		//	load usr operators.
		SharedLibrary	userOperatorLibrary;
		if(!(userOperatorLibrary.load(user_operator_library_path)))
			exit(-1);

		typedef	uint16	(*OpcodeRetriever)(const	char	*);
		typedef	void	(*UserInit)(OpcodeRetriever);
		UserInit	_Init=userOperatorLibrary.getFunction<UserInit>("Init");
		if(!_Init)
			return	false;

		typedef	bool	(*UserOperator)(const	Context	&,uint16	&);
		typedef	uint16	(*UserGetOperatorCount)();
		UserGetOperatorCount	GetOperatorCount=userOperatorLibrary.getFunction<UserGetOperatorCount>("GetOperatorCount");
		if(!GetOperatorCount)
			return	false;

		typedef	void	(*UserGetOperator)(UserOperator	&,char	*);
		UserGetOperator	GetOperator=userOperatorLibrary.getFunction<UserGetOperator>("GetOperator");
		if(!GetOperator)
			return	false;

		_Init(RetrieveOpcode);
		uint16	operatorCount=GetOperatorCount();
		for(uint16	i=0;i<operatorCount;++i){

			UserOperator	op;
			char			op_name[256];
			memset(op_name,0,256);
			GetOperator(op,op_name);

			UNORDERED_MAP<std::string,uint16>::iterator	it=_Opcodes.find(op_name);
			if(it==_Opcodes.end()){

				std::cerr<<"Operator "<<op_name<<" is undefined"<<std::endl;
				exit(-1);
			}
			Operator::Register(it->second,op);
		}

		std::cout<<"> User-defined operator library "<<user_operator_library_path<<" loaded"<<std::endl;

		return	true;
	}

	bool	Init(const	char	*user_operator_library_path,
				uint64			(*time_base)(),
				const	char	*seed_path){

		Now=time_base;

		std::string	error;
		if(!Compile(seed_path,error,true)){

			std::cerr<<error<<std::endl;
			return	false;
		}

		return	Init(user_operator_library_path,time_base);
	}

	bool	Init(const	char				*user_operator_library_path,
				uint64						(*time_base)(),
				const	r_comp::Metadata	&metadata,
				const	r_comp::Image		&seed){

		Metadata=metadata;
		Seed=seed;

		return	Init(user_operator_library_path,time_base);
	}

	uint16	GetOpcode(const	char	*name){

		UNORDERED_MAP<std::string,uint16>::iterator it=_Opcodes.find(name);
		if(it==_Opcodes.end())
			return	0xFFFF;
		return	it->second;
	}

	std::string	GetAxiomName(const	uint16	index){

		return	Compiler.getObjectName(index);
	}
}