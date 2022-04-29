# Architecture

**Compiler Architecture**
![compiler architecture](https://mermaid.ink/img/eyJjb2RlIjoiZmxvd2NoYXJ0IFREXG4gIHN1YmdyYXBoIGtub3QubGliXG4gICAgQVNUXG4gIGVuZFxuXG4gIHN1YmdyYXBoIGtub3QuYW5hbHl6ZVxuICAgIEFuYWx5emVyXG4gIGVuZFxuXG4gIHN1YmdyYXBoIGtub3QuZmlsZVxuICAgIElucHV0U3RyZWFtXG4gICAgQ2FjaGVcbiAgICBJT1xuICAgIFdyaXRlclxuXG4gICAgSW5wdXRTdHJlYW0gLS0-IElPXG4gIGVuZFxuXG4gIHN1YmdyYXBoIGtub3QuY29tcGlsZVxuICAgIENvbXBpbGVyXG4gICAgUGFyc2VyXG4gIGVuZFxuXG4gIHN1YmdyYXBoIGtub3QucmVzb2x2ZVxuICAgIFJlc29sdmVyXG4gICAgTW9kdWxlXG5cbiAgICBNb2R1bGUgLS0-IFJlc29sdmVyXG4gIGVuZFxuXG4gIHN1YmdyYXBoIGtub3QucGFyc2VcbiAgICBPbnl4XG4gIGVuZFxuXG4gIHN1YmdyYXBoIGtub3QuZ3JhbW1hclxuICAgIHN1YmdyYXBoIHBhcnNlcnNcbiAgICAgIFByb2dyYW1cbiAgICAgIERlY2xhcmF0aW9uXG5cbiAgICAgIERlY2xhcmF0aW9uIC0tPiBQcm9ncmFtXG4gICAgZW5kXG5cbiAgICBGb3JtYXR0ZXJcbiAgZW5kXG5cbiAgc3ViZ3JhcGgga25vdC5nZW5lcmF0ZVxuICAgIEdlbmVyYXRvclxuICBlbmRcblxuICBBU1QgLS0-IEFuYWx5emVyXG4gIEFTVCAtLT4ga25vdC5ncmFtbWFyXG5cbiAgQ2FjaGUgLS0-IFJlc29sdmVyXG4gIFdyaXRlciAtLT4gQ29tcGlsZXJcbiAgSU8gLS0-IE1vZHVsZVxuXG4gIE9ueXggLS0-IHBhcnNlcnNcblxuICBBbmFseXplciAtLT4gRGVjbGFyYXRpb25cblxuICBGb3JtYXR0ZXIgLS0-IEdlbmVyYXRvclxuICBQcm9ncmFtIC0tPiBQYXJzZXJcbiAgXG4gIFBhcnNlciAtLT4gQ29tcGlsZXJcbiAgUmVzb2x2ZXIgLS0-IENvbXBpbGVyXG5cbiAgR2VuZXJhdG9yIC0tPiBDb21waWxlciIsIm1lcm1haWQiOnsidGhlbWUiOiJkZWZhdWx0In0sInVwZGF0ZUVkaXRvciI6ZmFsc2UsImF1dG9TeW5jIjp0cnVlLCJ1cGRhdGVEaWFncmFtIjpmYWxzZX0)

**Binary Architecture**
![binary architecture](https://mermaid.ink/img/eyJjb2RlIjoiZmxvd2NoYXJ0IFREXG4gICUlIGZpbGVcbiAgc3ViZ3JhcGgga25vdC5maWxlXG4gICAgV2F0Y2hlclxuICBlbmRcbiAgV2F0Y2hlciAtLT4gV2F0Y2hcblxuICAlJSBjb21waWxlXG4gIHN1YmdyYXBoIGtub3QuY29tcGlsZVxuICAgIENvbXBpbGVyXG4gIGVuZFxuICBDb21waWxlciAtLT4gQnVpbGRcbiAgQ29tcGlsZXIgLS0-IEZvcm1hdFxuICBDb21waWxlciAtLT4gUnVudGltZVxuICBDb21waWxlciAtLT4gV2F0Y2hcblxuICAlJSBsYW5nX3NlcnZlclxuICBzdWJncmFwaCBrbm90Lmxhbmdfc2VydmVyXG4gICAgc3ViZ3JhcGggZXZlbnRfaGFuZGxlcnNcbiAgICAgIEluaXRpYWxpemVcbiAgICAgIENvZGVDb21wbGV0aW9uXG4gICAgICBHb1RvRGVmaW5pdGlvblxuICAgICAgSG92ZXJcbiAgICAgIExvY2FsU3ltYm9sc1xuICAgICAgV29ya3NwYWNlU3ltYm9sc1xuICAgIGVuZFxuICAgIFNlcnZlclxuICAgIFJ1bnRpbWVcblxuICAgIFJ1bnRpbWUgLS0-IFNlcnZlclxuICAgIGV2ZW50X2hhbmRsZXJzIC0tPiBTZXJ2ZXJcbiAgZW5kXG4gIFNlcnZlciAtLT4gTFNQXG5cbiAgJSUgZXhlY1xuICBzdWJncmFwaCBrbm90LmV4ZWNcbiAgICBzdWJncmFwaCBjb21tYW5kc1xuICAgICAgV2F0Y2hcbiAgICAgIEZvcm1hdFxuICAgICAgQnVpbGRcbiAgICAgIExTUFxuICAgICAgTGludFxuICAgICAgQnVuZGxlXG4gICAgICBEZXZlbG9wXG4gICAgZW5kXG4gIGVuZFxuICBjb21tYW5kcyAtLT4ga25vdFxuXG4gICUlIGNsaVxuICBzdWJncmFwaCBrbm90XG4gICAgTWFpblxuICBlbmRcbiIsIm1lcm1haWQiOnsidGhlbWUiOiJkZWZhdWx0In0sInVwZGF0ZUVkaXRvciI6ZmFsc2UsImF1dG9TeW5jIjp0cnVlLCJ1cGRhdGVEaWFncmFtIjpmYWxzZX0)

**Compilation Flow**
![compilation flow](https://mermaid.ink/img/eyJjb2RlIjoiZmxvd2NoYXJ0IExSXG4gIFh7e1wiI3F1b3Q7QC9teU1vZHVsZSNxdW90O1wifX1cblxuICBYIC0tPiB8aW5wdXR8IFJlc29sdmVyXG5cbiAgR3JhbW1hciAtLi0-IFBhcnNlclxuXG4gICUlIGNvbXBpbGVcbiAgc3ViZ3JhcGggQ29tcGlsZXJcbiAgICBSZXNvbHZlciAtLT4gTW9kdWxlIC0tPiBJbnB1dFN0cmVhbSAtLT4gUGFyc2VyXG4gIGVuZFxuICBQYXJzZXIgLS0-IEdlbmVyYXRvciAtLT4gV3JpdGVyIC0tPiBZKFsqKmNvbXBpbGVkIGNvZGUqKl0pXG5cbiAgQW5hbHl6ZXIgLS0-IFBhcnNlclxuXG4iLCJtZXJtYWlkIjp7InRoZW1lIjoiZGVmYXVsdCJ9LCJ1cGRhdGVFZGl0b3IiOmZhbHNlLCJhdXRvU3luYyI6dHJ1ZSwidXBkYXRlRGlhZ3JhbSI6ZmFsc2V9)

## knot.lib

___

### AST

Contains the type defintion for the abstract-syntax-tree that is constructed by the compiler.

## knot.analyze

___

### Analyzer

Resolves the types of fragments of the AST during semantic analysis.

## knot.parse

___

### Onyx

Recursive-descent parsing utility library.

## knot.file

___

### InputStream

Represents a stream of Unicode characters being read from a source.

### Cache

An interface for resolving files from a source, caching them and retrieving them from their cached location.

### IO

Utilities for interfacing with the file system and streams.

### Watcher

Observer that uses fswatch under the hood to watch file system events.

### Writer

Utilities for writing to the file system.

## knot.grammar

___

### Formatter

Opinionated formatter for converting ASTs into valid source code.

### Declaration

Parsing utilities for module declarations. This is where semantic analysis is invoked.

### Program

The entrypoint for grammar parsing utilities.

## knot.resolve

___

### Module

Represent a source code module with utilities for reading.

### Resolver

Finds source code modules in a file system or other source.

## knot.generate

___

### Generator

Transforms a valid AST into different compilation targets.

## knot.compile

___

### Parser

Transforms an InputStream into a valid AST.

### Compiler

Maintains the compiled artifacts for all loaded modules. Resolves dependencies and compiles source modules to a target.

## knot.language_server

___

### Runtime

Represents the active state of the language server.

### Server

The entrypoint for running a language server.

### Initialize

Handler for the `initialize` LSP event. Declares the capabilities of the language server.

### CodeCompletion

Handler for the `textDocument/completion` LSP event. Provides completion for a token at some point in a source document.

### GoToDefinition

Handler for the `textDocument/definition` LSP event. Find the location where a variable was declared.

### Hover

Handler for the `textDocument/hover` LSP event. Finds the type of the symbol at some point in a source document.

### LocalSymbols

Handler for the `textDocument/documentSymbol` LSP event. Find symbols in a source document at some point.

### WorkspaceSymbols

Handler for the `workspace/symbol` LSP event. Find symbols in other modules within the same project.

## knot.exec

___

### Build

Handler for the `knot build` CLI command. Compiles a program to some target.

### Watch

Handler for the `knot watch` CLI command. Compiles a program to some target then watches for changes and incrementally build the difference.

### Format

Handler for the `knot format` CLI command. Format the source files of a program using the official formatter.

### LSP

Handler for the `knot lsp` CLI command. Runs the official language server.

### Lint

Handler for the `knot lint` CLI command. Runs the official linter.

### Bundle

Handler for the `knot bundle` CLI command. Runs the official bundler.

### Develop

Handler for the `knot develop` CLI command. Runs the official development server.

## knot

___

### Main

The entrypoint for the `knot` binary.
