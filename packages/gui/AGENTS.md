# Configurações para Desenvolvimento Contínuo da Dashboard (View)

## Componentes de estilização:

A aplicação usa componentes da biblioteca Shadcn/ui, então todo o padrão tem que seguir o da mesma, ou de bibliotecas derivadas dela.
Link para a biblioteca: [Shadcn](https://ui.shadcn.com/).

### Adicionando um componente novo:

Apenas use o seguinte comando para adicionar:

```bash
pnpm dlx shadcn@latest add alert
```

Importe o componente na aplicação da seguinte forma:

```tsx
import { Alert, AlertDescription, AlertTitle } from '@/components/ui/alert'
```

E use o mesmo dentro do código:

```tsx
<Alert variant="default | destructive">
  <Terminal />
  <AlertTitle>Heads up!</AlertTitle>
  <AlertDescription>
    You can add components and dependencies to your app using the cli.
  </AlertDescription>
</Alert>
```

## Roteamento e criação de novas páginas na aplicação:

A aplicação usa o TanStack Router, que é uma biblioteca de roteamento por arquivos, todos estando na pasta `src/routes`.
Link para a biblioteca: [TanStack Router](https://tanstack.com/router).

### Adicionando uma rota:

Para adicionar uma nova rota na aplicação, apenas crie um novo arquivo na pasta `src/routes`, o TanStack vai entender automaticamente que o conteúdo naquela página é referente a uma rota, o nome do arquivo será o nome da rota.

No entanto, a partir do momento que temos duas rotas, é necessário usar um componente `Link` para poder navegar entre eles.

### Adicionando componentes Links:

Para fazer a navegação, vai ser necessário importar o componente `Link` de `@tanstack/react-router`.

```tsx
import { Link } from '@tanstack/react-router'
```

Então, em qualquer lugar do projeto
Then anywhere in your JSX you can use it like so:

```tsx
<Link to="/about">About</Link>
```

Isso vai criar um link que navega para a rota `/about`.

### Utilizando Layouts

No sistema de roteamento baseado em arquivos, o arquivo `src/routes/__root.tsx` é um arquivo para configuração de layout de rotas, tudo que você adicionar nesse arquivo, vai aparecer em todas as rotas. O conteúdo das rotas de fato, aparece onde você definir a tag `<Outlet />` no arquivo, por exemplo:

```tsx
import { Outlet, createRootRoute } from '@tanstack/react-router'

import { Link } from '@tanstack/react-router'

export const Route = createRootRoute({
  component: () => (
    <>
      <header>
        <nav>
          <Link to="/">Home</Link>
          <Link to="/about">About</Link>
        </nav>
      </header>
      <Outlet />
    </>
  ),
})
```

## Requisição de Dados:

Naturalmente, em Typescript existem diversas formas de fazer requisição de dados de APIs, mas nessa aplicação estamos usando o TanStack Query e também o `loader` padrão do TanStack Router para casos onde a informação precisa ser carregada antes da rota ser renderizada.

Exemplo de TanStack Query:

```tsx
import {
  QueryClient,
  QueryClientProvider,
  useQuery,
} from '@tanstack/react-query'

const queryClient = new QueryClient()

export default function App() {
  return (
    <QueryClientProvider client={queryClient}>
      <Example />
    </QueryClientProvider>
  )
}

function Example() {
  const { isPending, error, data } = useQuery({
    queryKey: ['repoData'],
    queryFn: () =>
      fetch('https://api.github.com/repos/TanStack/query').then((res) =>
        res.json(),
      ),
  })

  if (isPending) return 'Loading...'

  if (error) return 'An error has occurred: ' + error.message

  return (
    <div>
      <h1>{data.name}</h1>
      <p>{data.description}</p>
      <strong>👀 {data.subscribers_count}</strong>{' '}
      <strong>✨ {data.stargazers_count}</strong>{' '}
      <strong>🍴 {data.forks_count}</strong>
    </div>
  )
}
```

Exemplo de uso do `loader`:

```tsx
const peopleRoute = createRoute({
  getParentRoute: () => rootRoute,
  path: '/people',
  loader: async () => {
    const response = await fetch('https://swapi.dev/api/people')
    return response.json() as Promise<{
      results: {
        name: string
      }[]
    }>
  },
  component: () => {
    const data = peopleRoute.useLoaderData()
    return (
      <ul>
        {data.results.map((person) => (
          <li key={person.name}>{person.name}</li>
        ))}
      </ul>
    )
  },
})
```

Dessa forma, a aplicação fica padronizada para os momentos que precisa carregar os dados e os momentos que não precisa.

## Gerenciamento de Estados

Toda aplicação React precisar lidar com hooks para controle de estados da reatividade da página, nessa aplicação, estamos usando a TanStack Store.

Segue um exemplo de uso do Store no TanStack Store:

```tsx
import { useStore } from '@tanstack/react-store'
import { Store } from '@tanstack/store'
import './App.css'

const countStore = new Store(0)

function App() {
  const count = useStore(countStore)
  return (
    <div>
      <button onClick={() => countStore.setState((n) => n + 1)}>
        Increment - {count}
      </button>
    </div>
  )
}

export default App
```

Outra funcionalidade importante que é usada na aplicação, é a derivação de estados com o TanStack Store, onde a partir de um estado, modificamos outro com a função `Derived`.

Segue um exemplo de uso do Derived do TanStack Store:

```tsx
import { useStore } from '@tanstack/react-store'
import { Store, Derived } from '@tanstack/store'
import './App.css'

const countStore = new Store(0)

const doubledStore = new Derived({
  fn: () => countStore.state * 2,
  deps: [countStore],
})
doubledStore.mount()

function App() {
  const count = useStore(countStore)
  const doubledCount = useStore(doubledStore)

  return (
    <div>
      <button onClick={() => countStore.setState((n) => n + 1)}>
        Increment - {count}
      </button>
      <div>Doubled - {doubledCount}</div>
    </div>
  )
}

export default App
```
