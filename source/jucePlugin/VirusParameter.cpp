#include "VirusParameter.h"

#include "VirusController.h"

namespace Virus
{
	Parameter::Parameter(Controller &ctrl, const Description& desc, const uint8_t partNum, const int uniqueId) :
		RangedAudioParameter(genId(desc, partNum, uniqueId), "Ch " + juce::String(partNum + 1) + " " + desc.name), m_ctrl(ctrl),
		m_desc(desc), m_partNum(partNum), m_uniqueId(uniqueId)
	{
		m_range.start = static_cast<float>(m_desc.range.getStart());
		m_range.end = static_cast<float>(m_desc.range.getEnd());
		m_range.interval = m_desc.isDiscrete || m_desc.isBool ? 1.0f : 0.0f;
		m_value.addListener(this);
    }

    void Parameter::valueChanged(juce::Value &)
    {
		const uint8_t value = roundToInt(m_value.getValue());
		jassert (m_range.getRange().contains(value) || m_range.end == value);
		if (value != m_lastValue)
		{
			// ignore initial update
			if(m_lastValue != -1)
				m_ctrl.sendSysEx(m_ctrl.constructMessage({static_cast<uint8_t>(m_desc.page), m_partNum, m_desc.index, value}));
			m_lastValue = value;
		}
		if (onValueChanged)
			onValueChanged();
	}

    void Parameter::setLinkedValue(const int _value)
    {
		const int newValue = juce::roundToInt(m_range.getRange().clipValue(static_cast<float>(_value)));

		if (newValue == m_lastValue)
			return;

		m_lastValue = newValue;

		if(getDescription().isPublic)
		{
			beginChangeGesture();
			setValueNotifyingHost(convertTo0to1(static_cast<float>(newValue)));
			endChangeGesture();
		}
		else
		{
			m_value.setValue(newValue);
		}
	}

    bool Parameter::isMetaParameter() const
    {
	    return !m_linkedParameters.empty();
    }

    void Parameter::setValue(float newValue)
	{
		if (m_changingLinkedValues)
			return;

		m_value.setValue(convertFrom0to1(newValue));

		m_changingLinkedValues = true;

		for (const auto& parameter : m_linkedParameters)
		{
			if(!parameter->m_changingLinkedValues)
				parameter->setLinkedValue(m_value.getValue());
		}

		m_changingLinkedValues = false;
	}

	void Parameter::setValueFromSynth(int newValue, const bool notifyHost)
	{
		if (newValue == m_lastValue)
			return;

		m_lastValue = newValue;

		if (notifyHost && getDescription().isPublic)
		{
			beginChangeGesture();
			setValueNotifyingHost(convertTo0to1(static_cast<float>(newValue)));
			endChangeGesture();
		}
		else
		{
			m_value.setValue(newValue);
		}

		if (m_changingLinkedValues)
			return;

		m_changingLinkedValues = true;

		for (const auto& p : m_linkedParameters)
			p->setLinkedValue(newValue);

		m_changingLinkedValues = false;
	}

	juce::String Parameter::genId(const Description &d, const int part, const int uniqueId)
	{
		if(uniqueId > 0)
			return juce::String::formatted("%d_%d_%d_%d", static_cast<int>(d.page), part, d.index, uniqueId);
		return juce::String::formatted("%d_%d_%d", static_cast<int>(d.page), part, d.index);
	}

	void Parameter::addLinkedParameter(Parameter* _param)
	{
		if (_param == this)
			return;

		for (auto* p : m_linkedParameters)
		{
			_param->m_linkedParameters.insert(p);
			p->m_linkedParameters.insert(_param);
		}

		m_linkedParameters.insert(_param);
		_param->m_linkedParameters.insert(this);
	}
} // namespace Virus
