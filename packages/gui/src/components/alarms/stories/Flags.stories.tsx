import type { Meta, StoryObj } from '@storybook/react'
import { Flags } from '../page/Flags'

const meta = {
  title: 'Alarms/Flags',
  component: Flags,
  parameters: {
    layout: 'padded',
  },
  tags: ['autodocs'],
} satisfies Meta<typeof Flags>

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
