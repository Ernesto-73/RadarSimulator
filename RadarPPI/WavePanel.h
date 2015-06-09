#pragma once
#include "atltypes.h"
#include <vector>
#include "afxwin.h"

// WavePanel dialog

class WavePanel : public CDialogEx
{
	DECLARE_DYNAMIC(WavePanel)

public:
	WavePanel(CWnd* pParent = NULL);   // standard constructor
	virtual ~WavePanel();

// Dialog Data
	enum { IDD = IDD_WAVE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();
public:
	void Draw(CDC * pDC);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnPaint();
	CRect m_canvas;
	CRect m_Size;
	virtual BOOL OnInitDialog();
	void SetDataPointer(double * real, double * im, int size);
	double *m_real;
	double *m_im;
	void CalculateAbs(void);
	std::vector<double> m_abs;
	int m_size;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CFont m_font;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	double m_fEnergy;
	int m_iScale;
	BOOL m_bAutoDNR;
	BOOL m_bInherent;
};
