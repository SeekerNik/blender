/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Lukas Toenne
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef __BVM_CODEGEN_H__
#define __BVM_CODEGEN_H__

/** \file bvm_codegen.h
 *  \ingroup bvm
 */

#include <set>
#include <vector>

#include "MEM_guardedalloc.h"

#include "bvm_function.h"
#include "bvm_nodegraph.h"
#include "bvm_opcode.h"
#include "bvm_util_string.h"

namespace bvm {

struct Function;
struct NodeGraph;
struct NodeInstance;
struct TypeDesc;

typedef std::vector<const NodeInstance *> NodeList;
typedef std::set<const NodeInstance *> NodeSet;
typedef std::map<ConstInputKey, StackIndex> InputIndexMap;
typedef std::map<ConstOutputKey, StackIndex> OutputIndexMap;
typedef std::map<ConstOutputKey, int> SocketUserMap;

struct BasicBlock {
	BasicBlock() : entry_point(0), return_index(BVM_STACK_INVALID) {}
	NodeList nodes;
	InputIndexMap input_index;
	OutputIndexMap output_index;
	int entry_point;
	StackIndex return_index;
};
typedef std::map<ConstInputKey, BasicBlock> ExpressionMap;
typedef std::vector<int> StackUsers;

struct Compiler {
	
	Compiler();
	virtual ~Compiler();
	
protected:
	StackIndex find_stack_index(int size) const;
	StackIndex assign_stack_index(const TypeDesc &typedesc);
	
	void resolve_basic_block_symbols(const NodeGraph &graph, BasicBlock &block);
	void resolve_symbols(const NodeGraph &graph);
	
	virtual void push_opcode(OpCode op) const = 0;
	virtual void push_stack_index(StackIndex arg) const = 0;
	virtual void push_jump_address(int address) const = 0;
	
	virtual void push_float(float f) const = 0;
	virtual void push_float3(float3 f) const = 0;
	virtual void push_float4(float4 f) const = 0;
	virtual void push_int(int i) const = 0;
	virtual void push_matrix44(matrix44 m) const = 0;
	virtual void push_string(const char *s) const = 0;
	
	virtual int current_address() const = 0;
	
	void push_constant(const Value *value) const;
	
	void codegen_value(const Value *value, StackIndex offset) const;
	int codegen_basic_block(const BasicBlock &block,
	                        const SocketUserMap &socket_users) const;
	int codegen_main(const NodeGraph &graph);
	
	void expression_node_append(const NodeInstance *node, NodeList &sorted_nodes, NodeSet &visited);
	void graph_node_append(const NodeInstance *node, NodeList &sorted_nodes, NodeSet &visited);
	void sort_graph_nodes(const NodeGraph &graph);
	
	const BasicBlock &main_block() const { return main; }
	
private:
	BasicBlock main;
	ExpressionMap expression_map;
	StackUsers stack_users;

	MEM_CXX_CLASS_ALLOC_FUNCS("BVM:BVMCompiler")
};

struct BVMCompiler : public Compiler {
	BVMCompiler();
	~BVMCompiler();
	
	Function *compile_function(const NodeGraph &graph);
	
protected:
	void push_opcode(OpCode op) const;
	void push_stack_index(StackIndex arg) const;
	void push_jump_address(int address) const;
	
	void push_float(float f) const;
	void push_float3(float3 f) const;
	void push_float4(float4 f) const;
	void push_int(int i) const;
	void push_matrix44(matrix44 m) const;
	void push_string(const char *s) const;
	
	int current_address() const;
	
private:
	Function *fn;
};

struct DebugGraphvizCompiler : public Compiler {
	DebugGraphvizCompiler();
	~DebugGraphvizCompiler();
	
	void compile_function(const NodeGraph &graph, FILE *m_file, const string &label);
	
protected:
	void push_opcode(OpCode op) const;
	void push_stack_index(StackIndex arg) const;
	void push_jump_address(int address) const;
	
	void push_float(float m_file) const;
	void push_float3(float3 m_file) const;
	void push_float4(float4 m_file) const;
	void push_int(int i) const;
	void push_matrix44(matrix44 m) const;
	void push_string(const char *s) const;
	
	int current_address() const;
	
	const char *get_arg_name() const;
	bool is_arg_output() const;
	
	void init_graph(const string &label);
	void close_graph();
	
	void init_node();
	void close_node();
	
private:
	FILE *m_file;
	mutable int m_current_address;
	mutable const NodeType *m_current_opnode;
	mutable int m_current_arg;
};

} /* namespace bvm */

#endif /* __BVM_CODEGEN_H__ */
