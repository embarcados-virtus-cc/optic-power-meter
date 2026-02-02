import type { Meta, StoryObj } from '@storybook/react'
import { RxPower } from '../page/RxPower'

const meta = {
  title: 'Dashboard/RxPower',
  component: RxPower,
  parameters: {
    layout: 'padded',
  },
  tags: ['autodocs'],
} satisfies Meta<typeof RxPower>

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
