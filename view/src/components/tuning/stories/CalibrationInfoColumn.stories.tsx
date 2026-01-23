import type { Meta, StoryObj } from '@storybook/react'
import { CalibrationInfoColumn } from '../page/CalibrationInfoColumn'

const meta = {
  title: 'Tuning/CalibrationInfoColumn',
  component: CalibrationInfoColumn,
  parameters: {
    layout: 'padded',
  },
  tags: ['autodocs'],
} satisfies Meta<typeof CalibrationInfoColumn>

export default meta
type Story = StoryObj<typeof meta>

export const Default: Story = {}
