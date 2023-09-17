pub mod context;
mod declaration;
mod expression;
pub mod fragment;
mod infer;
mod ksx;
mod module;
mod parameter;
mod register;
mod statement;
mod type_expression;
use self::context::{AnalyzeContext, NodeContext, ScopeContext};
use crate::parser::{module::ModuleNode, position::Decrement};
use combine::Stream;
use context::FileContext;
use register::Register;
use std::{cell::RefCell, fmt::Debug};

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
    Module(Vec<(String, T)>),
}

#[derive(Clone, Debug, PartialEq)]
pub enum Weak {
    Unknown,
    Type(Type<usize>),
    Inherit(usize),
}

#[derive(Clone, Debug, PartialEq)]
pub enum RefKind {
    Type,
    Value,
}

pub type WeakRef = (RefKind, Weak);
pub type StrongRef = (RefKind, Type<usize>);

fn register_fragments<T>(
    x: ModuleNode<T, ()>,
    file_ctx: &RefCell<FileContext>,
) -> ModuleNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    x.register(&mut ScopeContext::new(file_ctx))
}

pub fn analyze<T>(x: ModuleNode<T, ()>) -> ModuleNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    // register AST fragments depth-first with monotonically increasing IDs
    let file_ctx = RefCell::new(FileContext::new());
    let untyped = register_fragments(x, &file_ctx);

    // apply weak type inference
    let mut analyze_ctx = AnalyzeContext::new(&file_ctx);
    infer::weak::infer_types(&mut analyze_ctx);

    // apply strong type inference
    infer::strong::infer_types(&mut analyze_ctx);

    untyped
}

#[cfg(test)]
mod tests {
    use super::fragment::Fragment;
    use crate::{
        analyzer::context::FileContext,
        parser::{
            expression::{primitive::Primitive, Expression},
            module::Module,
            statement::Statement,
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };
    use std::{cell::RefCell, collections::BTreeMap};
}
