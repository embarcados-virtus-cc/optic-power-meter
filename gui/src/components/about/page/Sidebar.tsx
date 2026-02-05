import { BookA, FileQuestionMark, School } from 'lucide-react'
import {
  Sheet,
  SheetContent,
  SheetDescription,
  SheetHeader,
  SheetTitle,
} from '@/components/ui/sheet'
import { Skeleton } from '@/components/ui/skeleton'

interface AboutProps {
  open: boolean
  onOpenChange: (open: boolean) => void
}

export function About({ open, onOpenChange }: AboutProps) {
  return (
    <Sheet open={open} onOpenChange={onOpenChange}>
      <SheetContent className="sm:max-w-xl w-full border-l border-border bg-background text-foreground shadow-2xl flex flex-col h-full">
        <SheetHeader className="mt-6 text-center">
          <SheetTitle className="flex text-xl font-medium justify-center gap-2 tracking-wide text-foreground">
            <FileQuestionMark className="mt-0.5" /> Sobre o Projeto
          </SheetTitle>
          <SheetDescription className="sr-only">
            Informações sobre o projeto, desenvolvedores e mentores.
          </SheetDescription>
        </SheetHeader>

        <div className="flex flex-1 flex-col gap-6 overflow-y-auto pb-10 px-6">
          {/* Project Description */}
          <div className="rounded-lg bg-secondary/50 p-6 text-sm text-justify leading-relaxed text-muted-foreground shadow-sm ring-1 ring-border/50">
            Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do
            eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim
            ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut
            aliquip ex ea commodo consequat. Duis aute irure dolor in
            reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla
            pariatur. Excepteur sint occaecat cupidatat non proident, sunt in
            culpa qui officia deserunt mollit anim id est laborum.
          </div>

          {/* Developers */}
          <div className="flex flex-col gap-4">
            <SheetTitle className="flex justify-center gap-2 text-xl font-medium text-center tracking-wide text-foreground my-2">
              <BookA className="mt-0.5" />
              Alunos Desenvolvedores:
            </SheetTitle>
            <div className="rounded-lg bg-secondary/50 p-6 shadow-sm ring-1 ring-border/50">
              <ul className="space-y-2 text-sm text-muted-foreground">
                <li>- Alexandre Basílio da Silva Junior (UFCG)</li>
                <li>- Carlos Elias Fialho de Lima (IFPB)</li>
                <li>- Miguel Ryan Dantas de Freitas (IFPB)</li>
                <li>- Melquisedeque Leite Silva (UFCG)</li>
                <li>- Nicholas Gomes da Costa (UFCG)</li>
                <li>- Pablo Daniel Nascimento Vieira (IFPB)</li>
                <li>- Pedro Augusto Gonçalves Lucena (IFPB)</li>
                <li>- Pedro Wilson Silva de Sousa (UFCG)</li>
              </ul>
            </div>
          </div>

          {/* Mentors */}
          <div className="flex flex-col gap-4">
            <SheetTitle className="flex justify-center gap-2 text-xl font-medium text-center tracking-wide text-foreground my-2">
              <School className="mt-0.5" />
              Professores Mentores:
            </SheetTitle>
            <div className="rounded-lg bg-secondary/50 p-6 shadow-sm ring-1 ring-border/50">
              <ul className="space-y-2 text-sm text-muted-foreground">
                <li>- Alexandre Sales Vasconcelos (IFPB)</li>
                <li>- Moacy Pereira da Silva (IFPB)</li>
                <li>- Rafael Bezerra Correia Lima (UFCG)</li>
              </ul>
            </div>
          </div>
        </div>
      </SheetContent>
    </Sheet>
  )
}

export function AboutSkeleton() {
  return (
    <div className="bg-background flex h-full w-full flex-col gap-4 p-6 border-l border-border">
      {/* Header Skeleton */}
      <div className="mb-4 flex flex-col gap-2">
        <Skeleton className="h-8 w-40 bg-muted/50" />
      </div>

      {/* Description Skeleton */}
      <div className="flex flex-col gap-2 rounded-lg bg-secondary/50 p-5 ring-1 ring-border/50">
        <Skeleton className="h-4 w-full bg-muted/50" />
        <Skeleton className="h-4 w-[90%] bg-muted/50" />
        <Skeleton className="h-4 w-[95%] bg-muted/50" />
        <Skeleton className="h-4 w-[85%] bg-muted/50" />
        <Skeleton className="h-4 w-full bg-muted/50" />
        <Skeleton className="h-4 w-[60%] bg-muted/50" />
      </div>

      {/* Developers Skeleton */}
      <div className="flex flex-col gap-3 rounded-lg bg-secondary/50 p-5 ring-1 ring-border/50">
        <Skeleton className="h-5 w-48 bg-muted/50" />
        <div className="space-y-2 pt-2">
          <Skeleton className="h-4 w-full bg-muted/50" />
          <Skeleton className="h-4 w-full bg-muted/50" />
          <Skeleton className="h-4 w-full bg-muted/50" />
          <Skeleton className="h-4 w-full bg-muted/50" />
          <Skeleton className="h-4 w-full bg-muted/50" />
          <Skeleton className="h-4 w-full bg-muted/50" />
        </div>
      </div>

      {/* Mentors Skeleton */}
      <div className="flex flex-col gap-3 rounded-lg bg-secondary/50 p-5 ring-1 ring-border/50">
        <Skeleton className="h-5 w-48 bg-muted/50" />
        <div className="space-y-2 pt-2">
          <Skeleton className="h-4 w-full bg-muted/50" />
          <Skeleton className="h-4 w-full bg-muted/50" />
          <Skeleton className="h-4 w-full bg-muted/50" />
        </div>
      </div>
    </div>
  )
}
