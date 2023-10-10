use super::import::Import;
use crate::{ast::declaration::DeclarationNode, common::position::Decrement};
use combine::Stream;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub struct Module<D> {
    pub imports: Vec<Import>,
    pub declarations: Vec<D>,
}

impl<D> Module<D> {
    pub fn new(imports: Vec<Import>, declarations: Vec<D>) -> Self {
        Self {
            imports,
            declarations,
        }
    }

    pub fn map<D2>(&self, fd: &impl Fn(&D) -> D2) -> Module<D2> {
        Module {
            imports: self.imports.clone(),
            declarations: self.declarations.iter().map(fd).collect(),
        }
    }

    pub fn is_empty(&self) -> bool {
        self.imports.is_empty() || self.declarations.is_empty()
    }
}

pub type ModuleNodeValue<T, C> = Module<DeclarationNode<T, C>>;

#[derive(Debug, PartialEq)]
pub struct ModuleNode<T, C>(pub ModuleNodeValue<T, C>, pub C)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> ModuleNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn map<R>(
        &self,
        f: impl Fn(&ModuleNodeValue<T, C>, &C) -> (ModuleNodeValue<T, R>, R),
    ) -> ModuleNode<T, R> {
        let (value, ctx) = f(&self.0, &self.1);

        ModuleNode(value, ctx)
    }
}

impl<T> ModuleNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: ModuleNodeValue<T, ()>) -> Self {
        Self(x, ())
    }
}
