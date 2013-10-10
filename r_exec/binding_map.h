//	binding_map.h
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

#ifndef	binding_map_h
#define	binding_map_h

#include	"object.h"
#include	"dll.h"


namespace	r_exec{

	class	BindingMap;
	class	AtomValue;
	class	StructureValue;
	class	ObjectValue;

	class	r_exec_dll	Value:
	public	_Object{
	protected:
		BindingMap	*map;
		Value(BindingMap	*map);
	public:
		virtual	Value	*copy(BindingMap	*map)	const=0;
		virtual	void	valuate(Code	*destination,uint16	write_index,uint16	&extent_index)	const=0;
		virtual	bool	match(const	Code	*object,uint16	index)=0;
		virtual	Atom	*get_code()=0;
		virtual	Code	*get_object()=0;
		virtual	uint16	get_code_size()=0;

		virtual	bool	intersect(const	Value	*v)				const{	return	false;	}
		virtual	bool	_intersect(const	AtomValue	*v)		const{	return	false;	}
		virtual	bool	_intersect(const	StructureValue	*v)	const{	return	false;	}
		virtual	bool	_intersect(const	ObjectValue	*v)		const{	return	false;	}

		virtual	bool	contains(const	Atom	a)	const{	return	false;	}
		virtual	bool	contains(const	Atom	*s)	const{	return	false;	}
		virtual	bool	contains(const	Code	*o)	const{	return	false;	}
	};

	class	r_exec_dll	BoundValue:
	public	Value{
	protected:
		BoundValue(BindingMap	*map);
	public:
	};

	class	r_exec_dll	UnboundValue:
	public	Value{
	private:
		uint8	index;
	public:
		UnboundValue(BindingMap	*map,uint8	index);
		~UnboundValue();

		Value	*copy(BindingMap	*map)	const;
		void	valuate(Code	*destination,uint16	write_index,uint16	&extent_index)	const;
		bool	match(const	Code	*object,uint16	index);
		Atom	*get_code();
		Code	*get_object();
		uint16	get_code_size();
	};

	class	r_exec_dll	AtomValue:
	public	BoundValue{
	private:
		Atom	atom;
	public:
		AtomValue(BindingMap	*map,Atom	atom);

		Value	*copy(BindingMap	*map)	const;
		void	valuate(Code	*destination,uint16	write_index,uint16	&extent_index)	const;
		bool	match(const	Code	*object,uint16	index);
		Atom	*get_code();
		Code	*get_object();
		uint16	get_code_size();

		bool	intersect(const	Value	*v)			const;
		bool	_intersect(const	AtomValue	*v)	const;

		bool	contains(const	Atom	a)	const;
	};

	class	r_exec_dll	StructureValue:
	public	BoundValue{
	private:
		P<Code>	structure;
		StructureValue(BindingMap	*map,const	Code	*structure);
	public:
		StructureValue(BindingMap	*map,const	Code	*source,uint16	structure_index);
		StructureValue(BindingMap	*map,Atom	*source,uint16	structure_index);
		StructureValue(BindingMap	*map,uint64	time);

		Value	*copy(BindingMap	*map)	const;
		void	valuate(Code	*destination,uint16	write_index,uint16	&extent_index)	const;
		bool	match(const	Code	*object,uint16	index);
		Atom	*get_code();
		Code	*get_object();
		uint16	get_code_size();

		bool	intersect(const	Value	*v)				const;
		bool	_intersect(const	StructureValue	*v)	const;

		bool	contains(const	Atom	*s)	const;
	};

	class	r_exec_dll	ObjectValue:
	public	BoundValue{
	private:
		const	P<Code>	object;
	public:
		ObjectValue(BindingMap	*map,Code	*object);

		Value	*copy(BindingMap	*map)	const;
		void	valuate(Code	*destination,uint16	write_index,uint16	&extent_index)	const;
		bool	match(const	Code	*object,uint16	index);
		Atom	*get_code();
		Code	*get_object();
		uint16	get_code_size();

		bool	intersect(const	Value	*v)			const;
		bool	_intersect(const	ObjectValue	*v)	const;

		bool	contains(const	Code	*o)	const;
	};

	typedef	enum{
		MATCH_SUCCESS_POSITIVE=0,
		MATCH_SUCCESS_NEGATIVE=1,
		MATCH_FAILURE=2
	}MatchResult;

	class	_Fact;
	class	Fact;

	class	r_exec_dll	BindingMap:
	public	_Object{
	friend	class	UnboundValue;
	protected:
		std::vector<P<Value> >	map;	// indexed by vl-ptrs.

		uint32	unbound_values;

		void	add_unbound_value(uint8	id);

		uint16	first_index;		// index of the first value found in the first fact.
		int16	fwd_after_index;	// tpl args (if any) are located before fwd_after_index.
		int16	fwd_before_index;

		bool	match_timings(uint64	stored_after,uint64	stored_before,uint64	after,uint64	before,uint32	destination_after_index,uint32	destination_before_index);
		bool	match_fwd_timings(const	_Fact	*f_object,const	_Fact	*f_pattern);
		bool	match(const	Code	*object,uint16	o_base_index,uint16	o_index,const	Code	*pattern,uint16	p_index,uint16	o_arity);

		void	abstract_member(Code	*object,uint16	index,Code	*abstracted_object,uint16	write_index,uint16	&extent_index);
		Atom	get_atom_variable(Atom	a);
		Atom	get_structure_variable(Code	*object,uint16	index);
		Atom	get_object_variable(Code	*object);
	public:
		BindingMap();
		BindingMap(const	BindingMap	*source);
		BindingMap(const	BindingMap	&source);
		virtual	~BindingMap();

		BindingMap&	operator	=(const	BindingMap	&source);
		void	load(const	BindingMap	*source);

		virtual	void	clear();

		void	init(Code	*object,uint16	index);

		_Fact	*abstract_f_ihlp(_Fact	*fact)	const;	// for icst and imdl.
		_Fact	*abstract_fact(_Fact	*fact,_Fact	*original,bool	force_sync);
		Code	*abstract_object(Code	*object,bool	force_sync);

		void	reset_fwd_timings(_Fact	*reference_fact);	// reset after and before from the timings of the reference object.

		MatchResult	match_fwd_lenient(const	_Fact	*f_object,const	_Fact	*f_pattern);	// use for facts when we are lenient about fact vs |fact.
		bool	match_fwd_strict(const	_Fact	*f_object,const	_Fact	*f_pattern);		// use for facts when we need sharp match.

		uint64	get_fwd_after()		const;	// assumes the timings are valuated.
		uint64	get_fwd_before()	const;	// idem.

		bool	match_object(const	Code	*object,const	Code	*pattern);
		bool	match_structure(const	Code	*object,uint16	o_base_index,uint16	o_index,const	Code	*pattern,uint16	p_index);
		bool	match_atom(Atom	o_atom,Atom	p_atom);

		void	bind_variable(BoundValue	*value,uint8	id);
		void	bind_variable(Atom	*code,uint8	id,uint16	value_index,Atom	*intermediate_results);

		Atom	*get_value_code(uint16	id);
		uint16	get_value_code_size(uint16	id);

		bool	intersect(BindingMap	*bm);
		bool	is_fully_specified()	const;

		Atom	*get_code(uint16	i)	const{	return	map[i]->get_code();	}
		Code	*get_object(uint16	i)	const{	return	map[i]->get_object();	}
		uint16	get_fwd_after_index()	const{	return	fwd_after_index;	}
		uint16	get_fwd_before_index()	const{	return	fwd_before_index;	}
		bool	scan_variable(uint16	id)	const;	// return true if id<first_index or map[id] is not an UnboundValue.
	};

	class	r_exec_dll	HLPBindingMap:
	public	BindingMap{
	private:
		int16	bwd_after_index;
		int16	bwd_before_index;

		bool	match_bwd_timings(const	_Fact	*f_object,const	_Fact	*f_pattern);

		bool	need_binding(Code	*pattern)	const;
		void	init_from_pattern(const	Code	*source,int16	position);	// first source is f->obj.
	public:
		HLPBindingMap();
		HLPBindingMap(const	HLPBindingMap	*source);
		HLPBindingMap(const	HLPBindingMap	&source);
		~HLPBindingMap();

		HLPBindingMap&	operator	=(const	HLPBindingMap	&source);
		void	load(const	HLPBindingMap	*source);
		void	clear();

		void	init_from_hlp(const	Code	*hlp);
		void	init_from_f_ihlp(const	_Fact	*f_ihlp);
		Fact	*build_f_ihlp(Code	*hlp,uint16	opcode,bool	wr_enabled)	const;	// return f->ihlp.
		Code	*bind_pattern(Code	*pattern)	const;

		void	reset_bwd_timings(_Fact	*reference_fact);	// idem for the last 2 unbound variables (i.e. timings of the second pattern in a mdl).

		MatchResult	match_bwd_lenient(const	_Fact	*f_object,const	_Fact	*f_pattern);	// use for facts when we are lenient about fact vs |fact.
		bool	match_bwd_strict(const	_Fact	*f_object,const	_Fact	*f_pattern);		// use for facts when we need sharp match.

		uint64	get_bwd_after()		const;	// assumes the timings are valuated.
		uint64	get_bwd_before()	const;	// idem.
	};
}


#endif
