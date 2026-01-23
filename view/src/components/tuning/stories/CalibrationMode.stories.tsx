import type { Meta, StoryObj } from '@storybook/react'
import { CalibrationMode } from '../page/CalibrationMode'
import { useState } from 'react'

const meta = {
  title: 'Tuning/CalibrationMode',
  component: CalibrationMode,
  parameters: {
    layout: 'padded',
  },
  tags: ['autodocs'],
} satisfies Meta<typeof CalibrationMode>

export default meta
type Story = StoryObj<typeof meta>

export const Default: Story = {
  args: {
    activeMode: 'internal',
    onModeChange: () => {},
    status: { rx: true, tx: true, bias: true, voltage: true, temp: true },
  },
  render: (args) => {
    const [mode, setMode] = useState(args.activeMode)
    return (
      <CalibrationMode {...args} activeMode={mode} onModeChange={setMode} />
    )
  },
}
