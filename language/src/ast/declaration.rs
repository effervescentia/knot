use super::{
    expression::ExpressionNode,
    module::ModuleNode,
    parameter::ParameterNode,
    storage::{Storage, Visibility},
    type_expression::TypeExpressionNode,
};
use crate::common::{node::Node, position::Decrement, range::Range};
use combine::Stream;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Declaration<E, P, M, T> {
    TypeAlias {
        name: Storage,
        value: T,
    },
    Constant {
        name: Storage,
        value_type: Option<T>,
        value: E,
    },
    Enumerated {
        name: Storage,
        variants: Vec<(String, Vec<T>)>,
    },
    Function {
        name: Storage,
        parameters: Vec<P>,
        body_type: Option<T>,
        body: E,
    },
    View {
        name: Storage,
        parameters: Vec<P>,
        body: E,
    },
    Module {
        name: Storage,
        value: M,
    },
}

impl<E, P, M, T> Declaration<E, P, M, T> {
    pub fn name(&self) -> &String {
        match self {
            Self::TypeAlias {
                name: Storage(_, name),
                ..
            }
            | Self::Enumerated {
                name: Storage(_, name),
                ..
            }
            | Self::Constant {
                name: Storage(_, name),
                ..
            }
            | Self::Function {
                name: Storage(_, name),
                ..
            }
            | Self::View {
                name: Storage(_, name),
                ..
            }
            | Self::Module {
                name: Storage(_, name),
                ..
            } => name,
        }
    }

    pub fn is_public(&self) -> bool {
        match self {
            Self::TypeAlias {
                name: Storage(visibility, _),
                ..
            }
            | Self::Enumerated {
                name: Storage(visibility, _),
                ..
            }
            | Self::Constant {
                name: Storage(visibility, _),
                ..
            }
            | Self::Function {
                name: Storage(visibility, _),
                ..
            }
            | Self::View {
                name: Storage(visibility, _),
                ..
            }
            | Self::Module {
                name: Storage(visibility, _),
                ..
            } => *visibility == Visibility::Public,
        }
    }

    pub fn map<E2, P2, M2, T2>(
        &self,
        fe: &impl Fn(&E) -> E2,
        fp: &impl Fn(&P) -> P2,
        fm: &impl Fn(&M) -> M2,
        ft: &impl Fn(&T) -> T2,
    ) -> Declaration<E2, P2, M2, T2> {
        match self {
            Self::TypeAlias { name, value } => Declaration::TypeAlias {
                name: name.clone(),
                value: ft(value),
            },

            Self::Enumerated { name, variants } => Declaration::Enumerated {
                name: name.clone(),
                variants: variants
                    .iter()
                    .map(|(name, parameters)| (name.clone(), parameters.iter().map(ft).collect()))
                    .collect(),
            },

            Self::Constant {
                name,
                value_type,
                value,
            } => Declaration::Constant {
                name: name.clone(),
                value_type: value_type.as_ref().map(ft),
                value: fe(value),
            },

            Self::Function {
                name,
                parameters,
                body_type,
                body,
            } => Declaration::Function {
                name: name.clone(),
                parameters: parameters.iter().map(fp).collect(),
                body_type: body_type.as_ref().map(ft),
                body: fe(body),
            },

            Self::View {
                name,
                parameters,
                body,
            } => Declaration::View {
                name: name.clone(),
                parameters: parameters.iter().map(fp).collect(),
                body: fe(body),
            },

            Self::Module { name, value } => Declaration::Module {
                name: name.clone(),
                value: fm(value),
            },
        }
    }
}

pub type DeclarationNodeValue<T, C> = Declaration<
    ExpressionNode<T, C>,
    ParameterNode<T, C>,
    ModuleNode<T, C>,
    TypeExpressionNode<T, C>,
>;

#[derive(Debug, PartialEq)]
pub struct DeclarationNode<T, C>(pub Node<DeclarationNodeValue<T, C>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> DeclarationNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn node(&self) -> &Node<DeclarationNodeValue<T, C>, T, C> {
        &self.0
    }

    pub fn map<R>(
        &self,
        f: impl Fn(&DeclarationNodeValue<T, C>, &C) -> (DeclarationNodeValue<T, R>, R),
    ) -> DeclarationNode<T, R> {
        let node = self.node();
        let (value, ctx) = f(&node.value(), &node.context());

        DeclarationNode(Node(value, node.range().clone(), ctx))
    }
}

impl<T> DeclarationNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: DeclarationNodeValue<T, ()>, range: Range<T>) -> Self {
        Self(Node::raw(x, range))
    }
}
