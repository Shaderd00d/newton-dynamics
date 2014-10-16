/* Copyright (c) <2009> <Newton Game Dynamics>
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "dCILstdafx.h"
#include "dCIL.h"
#include "dCILInstr.h"
#include "dDataFlowGraph.h"
#include "dCILInstrBranch.h"
#include "dRegisterInterferenceGraph.h"

dCILInstrLabel::dCILInstrLabel(dCIL& program, const dString& label)
	:dCILSingleArgInstr (program, dArg (label, dArgType()))
{
}

bool dCILInstrLabel::IsBasicBlockBegin() const
{
	return true;
}

dCILInstrLabel* dCILInstrLabel::GetAsLabel()
{
	return this;
}

void dCILInstrLabel::Serialize(char* const textOut) const
{
	sprintf (textOut, "%s:\n", m_arg0.m_label.GetStr());
}

dCILInstrGoto::dCILInstrGoto(dCIL& program, const dString& label)
	:dCILSingleArgInstr (program, dArg (label, dArgType()))
	,m_tagetNode(NULL)
{
}

bool dCILInstrGoto::IsBasicBlockEnd() const
{
	return true;
}

void dCILInstrGoto::Serialize(char* const textOut) const
{
	sprintf (textOut, "\tgoto %s\n", m_arg0.m_label.GetStr());
}

void dCILInstrGoto::SetTarget (dCILInstrLabel* const target)
{
	m_tagetNode = target->GetNode();
	dAssert (target->GetArg0().m_label == GetArg0().m_label);
}

dList<dCILInstr*>::dListNode* dCILInstrGoto::GetTarget () const
{
	return m_tagetNode;
}

dCILInstrGoto* dCILInstrGoto::GetAsGoto()
{
	return this;
}

dCILInstrIFNot::dCILInstrIFNot(dCIL& program, const dString& name, const dArgType& type, const dString& target0, const dString& target1)
	:dCILThreeArgInstr (program, dArg (name, type), dArg (target0, dArgType()), dArg (target1, dArgType()))
	,m_tagetNode0(NULL)
	,m_tagetNode1(NULL)
{
}

void dCILInstrIFNot::Serialize(char* const textOut) const
{
	if (m_tagetNode1) {
		sprintf(textOut, "\tifnot (%s %s) goto %s else goto %s\n", m_arg0.GetTypeName().GetStr(), m_arg0.m_label.GetStr(), m_arg1.m_label.GetStr(), m_arg2.m_label.GetStr());
	} else {
		sprintf(textOut, "\tifnot (%s %s) goto %s\n", m_arg0.GetTypeName().GetStr(), m_arg0.m_label.GetStr(), m_arg1.m_label.GetStr());
	}
}


void dCILInstrIFNot::SetTargets (dCILInstrLabel* const target0, dCILInstrLabel* const target1)
{
	dAssert(target0);
	dAssert (target0->GetArg0().m_label == GetArg1().m_label);
	m_tagetNode0 = target0->GetNode();

	if (target1) {
		dAssert(target1->GetArg0().m_label == GetArg2().m_label);
		m_tagetNode1 = target1->GetNode();
	} else {
		m_tagetNode1 = NULL;
	}
}

dList<dCILInstr*>::dListNode* dCILInstrIFNot::GetTrueTarget () const
{
	return m_tagetNode0;
}

dList<dCILInstr*>::dListNode* dCILInstrIFNot::GetFalseTarget () const
{
	return m_tagetNode1;
}


dCILInstrIFNot* dCILInstrIFNot::GetAsIF()
{
	return this;
}

bool dCILInstrIFNot::IsBasicBlockEnd() const
{
	return true;
}

void dCILInstrIFNot::AddGeneratedAndUsedSymbols (dDataFlowPoint& datFloatPoint) const
{
	dAssert (m_arg0.GetType().m_intrinsicType == m_int);
//	if (m_arg0.GetType().m_intrinsicType == m_int) {
	datFloatPoint.m_usedVariableSet.Insert (m_arg0.m_label);

//	} else {
//		dAssert (0);
//	}
}

void dCILInstrIFNot::AsignRegisterName(const dRegisterInterferenceGraph& interferenceGraph)
{
	dCILSingleArgInstr::AsignRegisterName(interferenceGraph);
}


void dCILInstrIFNot::EmitOpcode(dVirtualMachine::dOpCode* const codeOutPtr) const
{
	dVirtualMachine::dOpCode& code = codeOutPtr[m_byteCodeOffset];
	code.m_type2.m_opcode = unsigned(dVirtualMachine::m_bneq);
	code.m_type2.m_reg0 = RegisterToIndex(m_arg0.m_label);
	code.m_type2.m_imm2 = m_tagetNode0->GetInfo()->GetByteCodeOffset() - (m_byteCodeOffset + GetByteCodeSize()); 
}




dCILInstrReturn::dCILInstrReturn(dCIL& program, const dString& name, const dArgType& type)
	:dCILSingleArgInstr (program, dArg (name, type))
{
}

void dCILInstrReturn::Serialize(char* const textOut) const
{
	sprintf (textOut, "\tret %s %s\n", m_arg0.GetTypeName().GetStr(), m_arg0.m_label.GetStr());
}

bool dCILInstrReturn::IsBasicBlockEnd() const
{
	return true;
}


dCILInstrReturn* dCILInstrReturn::GetAsReturn()
{
	return this;
}

void dCILInstrReturn::AddGeneratedAndUsedSymbols (dDataFlowPoint& datFloatPoint) const
{
	switch (m_arg0.GetType().m_intrinsicType)
	{
		case m_int:
			//point.m_usedVariableSet.Insert(m_returnVariableName);
			datFloatPoint.m_usedVariableSet.Insert(m_arg0.m_label);
			break;

		case m_void:
		case m_constInt:
			break;

		default:	
			dAssert (0);
	}
}

void dCILInstrReturn::AsignRegisterName(const dRegisterInterferenceGraph& interferenceGraph)
{
	switch (m_arg0.GetType().m_intrinsicType)
	{
		case m_int:
			dCILSingleArgInstr::AsignRegisterName(interferenceGraph);
		break;

		case m_void:
		case m_constInt:
			break;

		default:
			dAssert(0);
	}
}

int dCILInstrReturn::GetByteCodeSize() const 
{ 
	switch (m_arg0.GetType().m_intrinsicType)
	{
		case m_constInt:
			return 2;

		case m_int:
			break;

		default:
			dAssert(0);
	}

	return 1; 
}

void dCILInstrReturn::EmitOpcode (dVirtualMachine::dOpCode* const codeOutPtr) const
{
	switch (m_arg0.GetType().m_intrinsicType)
	{
		case m_constInt:
		{
			dVirtualMachine::dOpCode& code = codeOutPtr[m_byteCodeOffset];
			code.m_type2.m_opcode = unsigned(dVirtualMachine::m_movi);
			code.m_type2.m_reg0 = D_RETURN_REGISTER_INDEX;
			code.m_type2.m_imm2 = m_arg0.m_label.ToInteger();
			break;
		}

		case m_int:
			break;

		default:
			dAssert(0);
	}


	dVirtualMachine::dOpCode& code = codeOutPtr[m_byteCodeOffset + 1];
	code.m_type2.m_opcode = unsigned(dVirtualMachine::m_ret);
	code.m_type2.m_reg0 = D_STACK_REGISTER_INDEX;
	code.m_type2.m_imm2 = 0;
}

/*
dCILInstrCall::dCILInstrCall(dCIL& program, const dString& name, const dArgType& type, dList<dArg>& parameters)
	:dCILSingleArgInstr (program, dArg (name, type))
{
	for (dList<dArg>::dListNode* node = parameters.GetFirst(); node; node = node->GetNext()) {
		m_parameters.Append (node->GetInfo());
	}
}
*/

dCILInstrCall::dCILInstrCall(dCIL& program, const dString& returnValue, const dArgType& type, const dString& target, dList<dArg>& parameters)
	:dCILTwoArgInstr (program, dArg (returnValue, type), dArg (target, dArgType()))
	,m_tagetNode(NULL)
{
	for (dList<dArg>::dListNode* node = parameters.GetFirst(); node; node = node->GetNext()) {
		m_parameters.Append (node->GetInfo());
	}
}

void dCILInstrCall::Serialize(char* const textOut) const
{
	if (m_arg0.m_isPointer || (m_arg0.m_intrinsicType != m_void)) {
		sprintf (textOut, "\t%s %s = call %s (", m_arg0.GetTypeName().GetStr(), m_arg0.m_label.GetStr(), m_arg1.m_label.GetStr());
	} else {
		sprintf (textOut, "\t%s call %s (", m_arg0.GetTypeName().GetStr(), m_arg1.m_label.GetStr());
	}

	for (dList<dArg>::dListNode* node = m_parameters.GetFirst(); node; node = node->GetNext()) { 
		char text[2048];
		const dArg& arg = node->GetInfo();
		if (node->GetNext()) {
			sprintf (text, "%s %s, ", arg.GetTypeName().GetStr(), arg.m_label.GetStr());
		} else {
			sprintf (text, "%s %s", arg.GetTypeName().GetStr(), arg.m_label.GetStr());
		}
		strcat (textOut, text);
	}
	strcat (textOut, ")\n");
}


void dCILInstrCall::AddGeneratedAndUsedSymbols (dDataFlowPoint& datFloatPoint) const
{
	if (m_arg0.GetType().m_isPointer || (m_arg0.GetType().m_intrinsicType != m_void)) {
		datFloatPoint.m_generatedVariable = m_arg0.m_label;
	}

	for (dList<dArg>::dListNode* node = m_parameters.GetFirst(); node; node = node->GetNext()) { 
		const dArg& arg = node->GetInfo();
		datFloatPoint.m_usedVariableSet.Insert (arg.m_label);
	}
}


void dCILInstrCall::AddDefinedVariable (dDefinedVariableDictionary& dictionary) const 
{
	if (m_arg0.GetType().m_isPointer || (m_arg0.GetType().m_intrinsicType != m_void)) {
		dDefinedVariableDictionary::dTreeNode* const node = dictionary.Insert (m_arg0.m_label);
		node->GetInfo().Append (m_myNode);
	}
}


void dCILInstrCall::AddKilledStatements(const dDefinedVariableDictionary& dictionary, dDataFlowPoint& datFloatPoint) const 
{ 
	if (m_arg0.GetType().m_isPointer || (m_arg0.GetType().m_intrinsicType != m_void)) {
		dCILInstr::AddKilledStatementLow (m_arg0, dictionary, datFloatPoint);
	}

	for (dList<dArg>::dListNode* node = m_parameters.GetFirst(); node; node = node->GetNext()) {
		const dArg& arg = node->GetInfo();
		dCILInstr::AddKilledStatementLow (arg, dictionary, datFloatPoint);
	}
}

void dCILInstrCall::AsignRegisterName(const dRegisterInterferenceGraph& interferenceGraph)
{
	if (m_arg0.GetType().m_isPointer || (m_arg0.GetType().m_intrinsicType != m_void)) {
		m_arg0.m_label = interferenceGraph.GetRegisterName(m_arg0.m_label);
	}

	for (dList<dArg>::dListNode* node = m_parameters.GetFirst(); node; node = node->GetNext()) {
		dArg& arg = node->GetInfo();
		arg.m_label = interferenceGraph.GetRegisterName(arg.m_label);
	}
}

void dCILInstrCall::EmitOpcode (dVirtualMachine::dOpCode* const codeOutPtr) const 
{
	dAssert (m_tagetNode);
	dVirtualMachine::dOpCode& code = codeOutPtr[m_byteCodeOffset];
	code.m_type2.m_opcode = unsigned(dVirtualMachine::m_calli);
	code.m_type2.m_reg0 = D_STACK_REGISTER_INDEX;
	code.m_type2.m_imm2 = m_tagetNode->GetInfo()->GetByteCodeOffset() - (m_byteCodeOffset + GetByteCodeSize()); 
}
