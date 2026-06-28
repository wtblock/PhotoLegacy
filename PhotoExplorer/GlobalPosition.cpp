/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "GlobalPosition.h"

/////////////////////////////////////////////////////////////////////////////
// output string formatted as: "degrees minutes seconds"
 CString CGlobalPosition::GetOutput()
{
	m_csOutput.Empty();
	if ( m_arrRawData.size() == 6 && m_arrRawData[ 5 ] != 0 )
	{
		const ULONG ulDegrees = m_arrRawData[ 0 ];
		const ULONG ulMinutes = m_arrRawData[ 2 ];
		const float fNumerator = (float)m_arrRawData[ 4 ];
		const float fDenominator = (float)m_arrRawData[ 5 ];
		const float fSeconds = fNumerator / fDenominator;
		m_csOutput.Format( L"%d deg %d' %f\"", ulDegrees, ulMinutes, fSeconds );
	}
	
	return m_csOutput;
} // GetOutput

/////////////////////////////////////////////////////////////////////////////

