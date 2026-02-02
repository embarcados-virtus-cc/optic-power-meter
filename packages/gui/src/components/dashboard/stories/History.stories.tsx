import type { Meta, StoryObj } from '@storybook/react'
import { History } from '../page/History'

const meta = {
  title: 'Dashboard/History',
  component: History,
  parameters: {
    layout: 'padded',
  },
  tags: ['autodocs'],
} satisfies Meta<typeof History>

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
