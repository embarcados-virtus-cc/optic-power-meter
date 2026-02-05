import type { Meta, StoryObj } from '@storybook/react'
import { Parameters } from '../page/Parameters'

const meta = {
  title: 'Dashboard/Parameters',
  component: Parameters,
  parameters: {
    layout: 'padded',
  },
  tags: ['autodocs'],
} satisfies Meta<typeof Parameters>

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
