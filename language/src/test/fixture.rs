use crate::{
    analyzer::{
        context::{AnalyzeContext, FileContext, ScopeContext},
        fragment::Fragment,
        WeakRef,
    },
    parser::{
        declaration::{Declaration, DeclarationNode},
        expression::{
            ksx::{KSXNode, KSX},
            statement::{Statement, StatementNode},
            Expression, ExpressionNode,
        },
        module::{Module, ModuleNode},
        node::Node,
        range::Range,
        types::type_expression::{TypeExpression, TypeExpressionNode},
        CharStream,
    },
};
use std::{cell::RefCell, collections::BTreeSet};

const RANGE: Range<CharStream> = Range::chars((1, 1), (1, 1));

type InitRange = ((i32, i32), (i32, i32));

pub fn f_ctx() -> RefCell<FileContext> {
    RefCell::new(FileContext::new())
}

pub fn f_ctx_from(xs: Vec<(usize, (Vec<usize>, Fragment))>) -> RefCell<FileContext> {
    let ctx = f_ctx();
    ctx.borrow_mut().fragments.extend(xs);
    ctx
}

pub fn s_ctx<'a>(file_ctx: &'a RefCell<FileContext>) -> ScopeContext<'a> {
    ScopeContext::new(file_ctx)
}

pub fn a_ctx_from<'a>(
    file: &'a RefCell<FileContext>,
    weak_refs: Vec<(usize, WeakRef)>,
    bindings: Vec<((Vec<usize>, String), BTreeSet<usize>)>,
) -> AnalyzeContext<'a> {
    let mut ctx = AnalyzeContext::new(file);
    ctx.weak_refs.extend(weak_refs);
    ctx.bindings.extend(bindings);
    ctx
}

/// node factories
pub mod n {
    use crate::parser::{
        declaration,
        expression::{self, ksx, statement},
        module,
        types::type_expression,
    };

    use super::*;

    pub fn x(
        x: expression::NodeValue<CharStream<'static>, ()>,
    ) -> ExpressionNode<CharStream<'static>, ()> {
        ExpressionNode(Node::new(x, RANGE, ()))
    }

    pub fn xr<'a>(
        x: expression::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> ExpressionNode<CharStream<'a>, ()> {
        ExpressionNode::raw(x, Range::chars(start, end))
    }

    pub fn xc<T>(
        x: expression::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> ExpressionNode<CharStream<'static>, T> {
        ExpressionNode(Node::new(x, RANGE, ctx))
    }

    pub fn s(
        x: statement::NodeValue<CharStream<'static>, ()>,
    ) -> StatementNode<CharStream<'static>, ()> {
        StatementNode(Node::new(x, RANGE, ()))
    }

    pub fn sr<'a>(
        x: statement::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> StatementNode<CharStream<'a>, ()> {
        StatementNode::raw(x, Range::chars(start, end))
    }

    pub fn sc<T>(
        x: statement::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> StatementNode<CharStream<'static>, T> {
        StatementNode(Node::new(x, RANGE, ctx))
    }

    pub fn kx(x: ksx::NodeValue<CharStream<'static>, ()>) -> KSXNode<CharStream<'static>, ()> {
        KSXNode(Node::new(x, RANGE, ()))
    }

    pub fn kxr<'a>(
        x: ksx::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> KSXNode<CharStream<'a>, ()> {
        KSXNode::raw(x, Range::chars(start, end))
    }

    pub fn kxc<T>(
        x: ksx::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> KSXNode<CharStream<'static>, T> {
        KSXNode(Node::new(x, RANGE, ctx))
    }

    pub fn tx(
        x: type_expression::NodeValue<CharStream<'static>, ()>,
    ) -> TypeExpressionNode<CharStream<'static>, ()> {
        TypeExpressionNode(Node::new(x, RANGE, ()))
    }

    pub fn txr<'a>(
        x: type_expression::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> TypeExpressionNode<CharStream<'a>, ()> {
        TypeExpressionNode::raw(x, Range::chars(start, end))
    }

    pub fn txc<T>(
        x: type_expression::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> TypeExpressionNode<CharStream<'static>, T> {
        TypeExpressionNode(Node::new(x, RANGE, ctx))
    }

    pub fn d(
        x: declaration::NodeValue<CharStream<'static>, ()>,
    ) -> DeclarationNode<CharStream<'static>, ()> {
        DeclarationNode(Node::new(x, RANGE, ()))
    }

    pub fn dr<'a>(
        x: declaration::NodeValue<CharStream<'a>, ()>,
        (start, end): InitRange,
    ) -> DeclarationNode<CharStream<'a>, ()> {
        DeclarationNode::raw(x, Range::chars(start, end))
    }

    pub fn dc<T>(
        x: declaration::NodeValue<CharStream<'static>, T>,
        ctx: T,
    ) -> DeclarationNode<CharStream<'static>, T> {
        DeclarationNode(Node::new(x, RANGE, ctx))
    }

    pub fn mr<'a>(x: module::NodeValue<CharStream<'a>, ()>) -> ModuleNode<CharStream<'a>, ()> {
        ModuleNode::raw(x)
    }
}

/// ast factories
pub mod a {
    use super::*;
    use crate::parser::declaration::{
        parameter::Parameter,
        storage::{Storage, Visibility},
    };

    pub fn type_<E, M, T>(name: &str, value: T) -> Declaration<E, M, T> {
        Declaration::TypeAlias {
            name: Storage(Visibility::Public, name.to_string()),
            value,
        }
    }

    pub fn enum_<E, M, T>(name: &str, variants: Vec<(String, Vec<T>)>) -> Declaration<E, M, T> {
        Declaration::Enumerated {
            name: Storage(Visibility::Public, name.to_string()),
            variants,
        }
    }

    pub fn const_<E, M, T>(name: &str, value_type: Option<T>, value: E) -> Declaration<E, M, T> {
        Declaration::Constant {
            name: Storage(Visibility::Public, name.to_string()),
            value_type,
            value,
        }
    }

    pub fn func_<E, M, T>(
        name: &str,
        parameters: Vec<Parameter<E, T>>,
        body_type: Option<T>,
        body: E,
    ) -> Declaration<E, M, T> {
        Declaration::Function {
            name: Storage(Visibility::Public, name.to_string()),
            parameters,
            body_type,
            body,
        }
    }

    pub fn view<E, M, T>(
        name: &str,
        parameters: Vec<Parameter<E, T>>,
        body: E,
    ) -> Declaration<E, M, T> {
        Declaration::View {
            name: Storage(Visibility::Public, name.to_string()),
            parameters,
            body,
        }
    }

    pub fn mod_<E, M, T>(name: &str, value: M) -> Declaration<E, M, T> {
        Declaration::Module {
            name: Storage(Visibility::Public, name.to_string()),
            value,
        }
    }
}
