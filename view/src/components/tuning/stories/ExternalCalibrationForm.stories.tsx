import type { Meta, StoryObj } from '@storybook/react'
import { ExternalCalibrationForm } from '../page/ExternalCalibrationForm'

const meta = {
  title: 'Tuning/ExternalCalibrationForm',
  component: ExternalCalibrationForm,
  parameters: {
    layout: 'padded',
  },
  tags: ['autodocs'],
} satisfies Meta<typeof ExternalCalibrationForm>

export default meta
type Story = StoryObj<typeof meta>

export const Default: Story = {
  args: {
    rxPower: { offset: '0.5', slope: '1.2', pwr4: '-1', pwr3: '0.1' },
    txPower: { offset: '0', slope: '1.01' },
    temperature: { offset: '-2', slope: '1.0' },
    info: {
      ddmType: 'Internal',
      version: '10.0',
      rxPowerType: 'Average',
      addressChange: 'Required',
    },
  },
}
