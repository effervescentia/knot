mod declaration;
mod expression;
mod import;
mod ksx;
mod module;
mod operator;
mod parameter;
mod shape;
mod statement;
pub mod storage;
mod type_expression;

use crate::Node;
pub use declaration::{Declaration, DeclarationNode, DeclarationNodeValue};
pub use expression::{Expression, ExpressionNode, ExpressionNodeValue, Primitive};
pub use import::{Import, ImportNode, ImportNodeValue, ImportSource};
pub use ksx::{KSXNode, KSXNodeValue, KSX};
pub use module::{Module, ModuleNode, ModuleNodeValue};
pub use operator::{BinaryOperator, UnaryOperator};
pub use parameter::{Parameter, ParameterNode, ParameterNodeValue};
pub use shape::{
    DeclarationShape, ExpressionShape, ImportShape, KSXShape, ModuleShape, ParameterShape,
    ProgramShape, StatementShape, ToShape, TypeExpressionShape,
};
pub use statement::{Statement, StatementNode, StatementNodeValue};
pub use type_expression::{TypeExpression, TypeExpressionNode, TypeExpressionNodeValue};

pub trait AstNode<V, R, C>
where
    R: Copy,
{
    fn new(value: V, range: R, ctx: C) -> Self;

    fn raw<O>(value: V, range: R) -> O
    where
        O: AstNode<V, R, ()>,
    {
        AstNode::new(value, range, ())
    }

    fn node(&self) -> &Node<V, R, C>;

    fn map<V2, C2, O>(&self, f: impl Fn(&V, &C) -> (V2, C2)) -> O
    where
        O: AstNode<V2, R, C2>,
    {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        AstNode::new(value, *node.range(), ctx)
    }
}

#[derive(Debug, PartialEq)]
pub struct TypedNode<V, R, C>(Node<V, R, C>);

impl<V, R, C> AstNode<V, R, C> for TypedNode<V, R, C>
where
    R: Copy,
{
    fn new(value: V, range: R, ctx: C) -> Self {
        Self(Node(value, range, ctx))
    }

    fn node(&self) -> &Node<V, R, C> {
        &self.0
    }
}
