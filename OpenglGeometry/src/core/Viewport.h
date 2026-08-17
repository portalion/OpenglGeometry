#pragma once

struct ViewportData
{
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
};

class Viewport
{
private:
	ViewportData m_Data;
	bool m_Dirty;
public:
	inline bool IsDirty() { return m_Dirty; }
	inline bool IsValid() const { return m_Data.width > 0 && m_Data.height > 0; }
	inline ViewportData GetData() const { return m_Data; }

	float Aspect() const;
	void Change(ViewportData newData);
	void Apply();
};
