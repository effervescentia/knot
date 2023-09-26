pub mod context;
mod declaration;
mod expression;
pub mod fragment;
pub mod infer;
mod ksx;
mod module;
mod parameter;
mod register;
mod statement;
mod type_expression;

use crate::{ast::module::ModuleNode, common::position::Decrement};
use combine::Stream;
use context::{FileContext, NodeContext, ScopeContext};
use infer::strong::{Strong, ToStrong};
use register::Register;
use std::{cell::RefCell, fmt::Debug};

use self::context::WeakContext;

#[derive(Clone, Debug, PartialEq)]
pub enum Type<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,
    Enumerated(Vec<(String, Vec<T>)>),
    Function(Vec<T>, T),
    View(Vec<T>, T),
    Module(Vec<(String, RefKind, T)>),
}

#[derive(Clone, Debug, PartialEq)]
pub enum RefKind {
    Type,
    Value,
    Mixed,
}

fn register_fragments<T>(x: ModuleNode<T, ()>) -> (ModuleNode<T, NodeContext>, FileContext)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let file_ctx = RefCell::new(FileContext::new());
    let untyped = x.register(&mut ScopeContext::new(&file_ctx));

    (untyped, file_ctx.into_inner())
}

pub fn analyze<T>(x: ModuleNode<T, ()>) -> ModuleNode<T, Strong>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    // register AST fragments depth-first with monotonically increasing IDs
    let (untyped, file_ctx) = register_fragments(x);

    // apply weak type inference
    let WeakContext {
        bindings,
        fragments,
        weak_refs,
    } = infer::weak::infer_types(file_ctx);

    // apply strong type inference
    let nodes = fragments.into_descriptors(weak_refs);
    let strong_ctx = infer::strong::infer_types(&nodes, bindings);

    untyped.to_strong(&strong_ctx)
}

#[cfg(test)]
mod tests {
    // TODO
}
