import type { Meta, StoryObj } from '@storybook/react'
import { Limits } from '../page/Limits'

const meta = {
  title: 'Alarms/Limits',
  component: Limits,
  parameters: {
    layout: 'padded',
  },
  tags: ['autodocs'],
} satisfies Meta<typeof Limits>

export default meta
type Story = StoryObj<typeof meta>

export const Default: Story = {
  args: {
    isLoading: false,
  },
}

export const Loading: Story = {
  args: {
    isLoading: true,
  },
}
