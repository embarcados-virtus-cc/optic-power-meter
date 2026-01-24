import type { Meta, StoryObj } from '@storybook/react'
import { NotificationToast } from '../page/NotificationToast'
import type { AlarmFlag } from '@/stores/alarmStore'

const meta = {
  title: 'Alarms/NotificationToast',
  component: NotificationToast,
  parameters: {
    layout: 'centered',
  },
  tags: ['autodocs'],
  argTypes: {
    onDismiss: { action: 'dismissed' },
  },
} satisfies Meta<typeof NotificationToast>

export default meta
type Story = StoryObj<typeof meta>

const mockFlagHigh: AlarmFlag = {
  id: '1',
  parameter: 'Temperatura',
  value: 50.5,
  unit: '°C',
  type: 'ALTO',
  timestamp: '12:00:00',
}

const mockFlagLow: AlarmFlag = {
  id: '2',
  parameter: 'Tensão',
  value: 2.8,
  unit: 'V',
  type: 'BAIXO',
  timestamp: '12:00:05',
}

export const AlarmHigh: Story = {
  args: {
    flag: mockFlagHigh,
  },
}

export const AlarmLow: Story = {
  args: {
    flag: mockFlagLow,
  },
}
