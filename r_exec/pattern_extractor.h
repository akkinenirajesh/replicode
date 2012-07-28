//	pattern_extractor.h
//
//	Author: Eric Nivel
//
//	BSD license:
//	Copyright (c) 2010, Eric Nivel
//	All rights reserved.
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are met:
//
//   - Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   - Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   - Neither the name of Eric Nivel nor the
//     names of their contributors may be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
//	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//	DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
//	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef	pattern_extractor_h
#define	pattern_extractor_h

#include	"../r_code/time_buffer.h"

#include	"binding_map.h"
#include	"guard_builder.h"
#include	"cst_controller.h"


namespace	r_exec{

	class	AutoFocusController;

	class	Input{
	public:
		P<BindingMap>	bindings;	// contains the values for the abstraction.
		P<_Fact>		abstraction;
		P<_Fact>		input;
		bool			eligible_cause;
		uint64			ijt;	// injection time.

		Input(View	*input,_Fact	*abstraction,BindingMap	*bindings):input(input->object),ijt(input->get_ijt()),eligible_cause(IsEligibleCause(input)),abstraction(abstraction),bindings(bindings){}
		Input():input(NULL),eligible_cause(false),abstraction(NULL),bindings(NULL),ijt(0){}
		Input(const	Input	&original):input(original.input),eligible_cause(original.eligible_cause),abstraction(original.abstraction),bindings(original.bindings),ijt(original.ijt){}

		static	bool	IsEligibleCause(r_exec::View	*view);

		bool	is_invalidated(uint64	time_reference,uint32	thz)	const{	// for storage in time_buffers.
			
			return	(time_reference-ijt>thz);
		}
	};

	class	CInput{	// cached inputs.
	public:
		P<BindingMap>	bindings;	// contains the values for the abstraction.
		P<_Fact>		abstraction;
		P<View>			input;
		bool			injected;
		uint64			ijt;	// injection time.
		CInput(View	*input,_Fact	*abstraction,BindingMap	*bindings):input(input),abstraction(abstraction),bindings(bindings),injected(false),ijt(input->get_ijt()){}
		CInput():input(NULL),abstraction(NULL),bindings(NULL),injected(false),ijt(0){}

		bool	is_invalidated(uint64	time_reference,uint32	thz)	const{	// for storage in time_buffers.
			
			return	(time_reference-ijt>thz);
		}

		bool	operator	==(const	CInput	&i)	const{	return	input==i.input;	}
	};

	// Targeted Pattern eXtractor.
	// Does nothing.
	// Used for wr_enabled productions, for well-rated productions or when model acqusiiton is disabled.
	class	r_exec_dll	TPX:
	public	_Object{
	protected:
		AutoFocusController	*auto_focus;
		P<_Fact>			target;	// goal or prediction target, or premise (CTPX); abstraction: lhs of a mdl for goals, rhs for predictions, premise for CTPX.
		P<BindingMap>		target_bindings;
		P<_Fact>			abstracted_target;

		P<CSTController>	cst_hook;	// in case the target is an icst.

		std::vector<P<BindingMap> >	new_maps;	// acquired (in the case the target's bm is not fully specified) while matching the target's bm with inputs.

		bool	filter(View	*input,_Fact	*abstracted_input,BindingMap	*bm);
		
		TPX(AutoFocusController	*auto_focus,_Fact	*target);
	public:
		TPX(AutoFocusController	*auto_focus,_Fact	*target,_Fact	*pattern,BindingMap	*bindings);
		virtual	~TPX();

		_Fact		*get_pattern()	const{	return	abstracted_target;	}
		BindingMap	*get_bindings()	const{	return	target_bindings;	}

		virtual	bool	take_input(View	*view,_Fact	*abstracted_input,BindingMap	*bm);
		virtual	void	signal(View	*input)	const;
		virtual	void	ack_pred_success(_Fact	*predicted_f);
	};

	class	ICST;

	class	r_exec_dll	_TPX:
	public	TPX{
	private:
		static	const	uint32	InputsInitialSize=16;
	protected:
		class	Component{	// for building csts.
		public:
			_Fact	*object;
			bool	discarded;
			Component(){}
			Component(_Fact	*object):object(object),discarded(false){}
		};

		r_code::list<Input>		inputs;	// time-controlled buffer (inputs older than tpx_time_horizon from now are discarded).
		std::vector<P<Code> >	mdls;	// new mdls.
		std::vector<P<Code> >	csts;	// new csts.
		std::vector<P<_Fact> >	icsts;	// new icsts.

		void	filter_icst_components(ICST	*icst,uint32	icst_index,std::vector<Component>	&components);
		_Fact	*_find_f_icst(_Fact	*component,uint16	&component_index);
		_Fact	*find_f_icst(_Fact	*component,uint16	&component_index);
		_Fact	*find_f_icst(_Fact	*component,uint16	&component_index,Code	*&cst);
		Code	*build_cst(const	std::vector<Component>	&components,BindingMap	*bm,_Fact	*main_component);

		Code	*build_mdl_head(HLPBindingMap	*bm,uint16	tpl_arg_count,_Fact	*lhs,_Fact	*rhs,uint16	&write_index);
		void	build_mdl_tail(Code	*mdl,uint16	write_index);

		void	inject_hlps()	const;
		void	inject_hlps(uint64	analysis_starting_time);

		virtual	std::string	get_header()	const=0;

		_TPX(AutoFocusController	*auto_focus,_Fact	*target,_Fact	*pattern,BindingMap	*bindings);
		_TPX(AutoFocusController	*auto_focus,_Fact	*target);
	public:
		virtual	~_TPX();

		void	debug(View	*input){};
	};

	// Pattern extractor targeted at goal successes.
	// Possible causes are younger than the production of the goal.
	// Models produced are of the form: M1[cause -> goal_target], where cause can be an imdl and goal_target can be an imdl.
	// M1 does not have template arguments.
	// Commands are ignored (CTPX' job).
	class	r_exec_dll	GTPX:	// target is the goal target.
	public	_TPX{
	private:
		P<Fact>	f_imdl;	// that produced the goal.

		std::vector<P<_Fact> >	predictions;	// successful predictions that may invalidate the need for model building.

		bool	build_mdl(_Fact	*cause,_Fact	*consequent,GuardBuilder	*guard_builder,uint64	period);
		bool	build_mdl(_Fact	*f_icst,_Fact	*cause_pattern,_Fact	*consequent,GuardBuilder	*guard_builder,uint64	period,Code	*new_cst);

		std::string	get_header()	const;
	public:
		GTPX(AutoFocusController	*auto_focus,_Fact	*target,_Fact	*pattern,BindingMap	*bindings,Fact	*f_imdl);
		~GTPX();

		bool	take_input(View	*input,_Fact	*abstracted_input,BindingMap	*bm);
		void	signal(View	*input)	const;
		void	ack_pred_success(_Fact	*predicted_f);
		void	reduce(View	*input);	// input is v->f->success(target,input) or v->|f->success(target,input).
	};

	// Pattern extractor targeted at prediciton failures.
	// Possible causes are older than the production of the prediction.
	// Models produced are of the form: M1[cause -> |imdl M0] where M0 is the model that produced the failed prediction and cause can be an imdl.
	// M1 does not have template arguments.
	// Commands are ignored (CTPX' job).
	class	r_exec_dll	PTPX:	// target is the prediction.
	public	_TPX{
	private:
		P<Fact>	f_imdl;	// that produced the prediction (and for which the PTPX will find strong requirements).

		bool	build_mdl(_Fact	*cause,_Fact	*consequent,GuardBuilder	*guard_builder,uint64	period);
		bool	build_mdl(_Fact	*f_icst,_Fact	*cause_pattern,_Fact	*consequent,GuardBuilder	*guard_builder,uint64	period,Code	*new_cst);

		std::string	get_header()	const;
	public:
		PTPX(AutoFocusController	*auto_focus,_Fact	*target,_Fact	*pattern,BindingMap	*bindings,Fact	*f_imdl);
		~PTPX();

		void	signal(View	*input)	const;
		void	reduce(View	*input);	// input is v->f->success(target,input) or v->|f->success(target,input).
	};

	// Pattern extractor targeted at changes of repeated input facts (SYMC_PERIODIC or SYNC_HOLD).
	// Models produced are of the form: [premise -> [cause -> consequent]], i.e. M1:[premise -> imdl M0], M0:[cause -> consequent].
	// M0 has template args, i.e the value of the premise and its after timestamp.
	// N.B.: before-after=upr of the group the input comes from.
	// The Consequent is a value different from the expected repetition of premise.
	// The premise is an icst assembled from inputs synchronous with the input expected to repeat.
	// Guards on values (not only on timings) are computed: this is the only TPX that does so.
	// Inputs with SYNC_HOLD: I/O devices are expected to send changes on such inputs as soon as possible.
	class	CTPX:
	public	_TPX{
	private:
		bool	stored_premise;
		P<View>	premise;

		GuardBuilder	*get_default_guard_builder(_Fact	*cause,_Fact	*consequent,uint64	period);
		GuardBuilder	*find_guard_builder(_Fact	*cause,_Fact	*consequent,uint64	period);

		bool	build_mdl(_Fact	*cause,_Fact	*consequent,GuardBuilder	*guard_builder,uint64	period);
		bool	build_mdl(_Fact	*f_icst,_Fact	*cause_pattern,_Fact	*consequent,GuardBuilder	*guard_builder,uint64	period);

		bool	build_requirement(HLPBindingMap	*bm,Code	*m0,uint64	period);

		std::string	get_header()	const;
	public:
		CTPX(AutoFocusController	*auto_focus,View	*premise);
		~CTPX();

		void	store_input(r_exec::View	*input);
		void	reduce(r_exec::View	*input);	// asynchronous: build models of value change if not aborted asynchronously by ASTControllers.
		void	signal(r_exec::View	*input);	// spawns mdl/cst building (reduce()).
	};
}


#endif