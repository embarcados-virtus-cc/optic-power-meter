import { Store } from '@tanstack/store'
import { api, type ContainerInfo } from '@/lib/api'

export const containersStore = new Store<ContainerInfo[]>([])
export const containerStatusStore = new Store({ loading: true, error: null as string | null })

async function refreshContainers() {
  try {
    const containers = await api.containers()
    containersStore.setState(containers)
    containerStatusStore.setState({ loading: false, error: null })
  } catch (err) {
    containerStatusStore.setState({
      loading: false,
      error: err instanceof Error ? err.message : String(err),
    })
  }
}

void refreshContainers()
setInterval(() => void refreshContainers(), 5000)
