#include "Engine/GKDBConstraint.h"

using namespace GKENGINE;

void GKENGINE::GKTabConstraint::SetPrimaryKey( const GKString& IN strPrimaryKey )
{
	m_vPrimaryKeys.push_back(strPrimaryKey);
}

std::vector<GKString> GKENGINE::GKTabConstraint::GetPrimaryKeys() const
{
	return m_vPrimaryKeys;
}

void GKENGINE::GKTabConstraint::ClearPrimaryKeys()
{
	m_vPrimaryKeys.clear();
}

void GKENGINE::GKTabConstraint::SetUnique( std::vector<GKString> vUniqueCols)
{
	m_vvUniques.push_back(vUniqueCols);
}

std::vector<std::vector<GKString> > GKENGINE::GKTabConstraint::GetUnique() const
{
	return m_vvUniques;
}

void GKENGINE::GKTabConstraint::ClearUnique()
{
	m_vvUniques.clear();
}

void GKENGINE::GKTabConstraint::Clear()
{
	ClearPrimaryKeys();
	ClearUnique();
}

GKENGINE::GKColConstraint::GKColConstraint():m_uOptions(0) {
}

void GKENGINE::GKColConstraint::SetNotNull()
{
	m_uOptions |= GKColConstraint::GKNOTNULL;
}

void GKENGINE::GKColConstraint::SetDefault( const GKString& strDefault )
{
	m_uOptions |= GKColConstraint::GKDEFVAL;
	m_strDefValue = strDefault;
}

GKString GKENGINE::GKColConstraint::GetDefault() const
{
	return m_strDefValue;
}

void GKENGINE::GKColConstraint::SetCheck( const GKString& strCheck )
{
	m_uOptions |= GKColConstraint::GKCHECK;
	m_strCheck = strCheck;
}

GKString GKENGINE::GKColConstraint::GetCheck() const
{
	return m_strCheck;
}

void GKENGINE::GKColConstraint::SetForeignKey( const GKString& strForeignKey )
{
	m_uOptions |= GKColConstraint::GKFORKEY;
	m_strForeignTable = strForeignKey;
}

GKString GKENGINE::GKColConstraint::GetForeignKey() const
{
	return m_strForeignTable;
}

GKbool GKENGINE::GKColConstraint::IsNull() const
{
	return GKColConstraint::GKNOTNULL == m_uOptions & GKColConstraint::GKNOTNULL;
}

GKbool GKENGINE::GKColConstraint::IsCheck() const
{
	return GKColConstraint::GKCHECK == m_uOptions & GKColConstraint::GKCHECK;
}

GKbool GKENGINE::GKColConstraint::IsDefaultValue() const
{
	return GKColConstraint::GKDEFVAL == m_uOptions & GKColConstraint::GKDEFVAL;
}

GKbool GKENGINE::GKColConstraint::IsForeignKey() const
{
	return GKColConstraint::GKFORKEY == m_uOptions & GKColConstraint::GKFORKEY;
}

void GKENGINE::GKColConstraint::Clear()
{
	m_uOptions = 0;
	m_strForeignTable.Empty();
	m_strCheck.Empty();
	m_strDefValue.Empty();
	m_strDefValue.Empty();
}