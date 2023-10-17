use crate::{
    analyzer::{
        context::{BindingMap, FileContext, FragmentMap, ScopeContext, StrongContext},
        fragment::Fragment,
        infer::strong::StrongRef,
    },
    ast::{
        Declaration, DeclarationNode, ExpressionNode, KSXNode, ModuleNode, StatementNode,
        TypeExpressionNode,
    },
    common::{node::Node, range::Range},
};
use std::{
    cell::RefCell,
    collections::{BTreeMap, BTreeSet, HashMap},
};

#[allow(dead_code)]
const RANGE: Range = Range((1, 1), (1, 1));

#[allow(dead_code)]
type InitRange = ((i32, i32), (i32, i32));

#[allow(dead_code)]
pub fn file_ctx() -> RefCell<FileContext> {
    RefCell::new(FileContext::new())
}

#[allow(dead_code)]
pub fn file_ctx_from(xs: Vec<(usize, (Vec<usize>, Fragment))>) -> FileContext {
    let ctx = file_ctx();
    ctx.borrow_mut().fragments.0.extend(xs);
    ctx.into_inner()
}

#[allow(dead_code)]
pub fn scope_ctx<'a>(file_ctx: &'a RefCell<FileContext>) -> ScopeContext<'a> {
    ScopeContext::new(file_ctx)
}

#[allow(dead_code)]
pub fn strong_ctx_from(
    fragments: Vec<(usize, (Vec<usize>, Fragment))>,
    refs: Vec<(usize, StrongRef)>,
    bindings: Vec<((Vec<usize>, String), BTreeSet<usize>)>,
) -> StrongContext {
    let mut ctx = StrongContext::new(
        FragmentMap(BTreeMap::from_iter(fragments)),
        BindingMap(HashMap::from_iter(bindings)),
    );
    ctx.refs.extend(refs);
    ctx
}

/// node factories
#[allow(dead_code)]
pub mod n {
    use super::*;
    use crate::ast::{
        DeclarationNodeValue, ExpressionNodeValue, KSXNodeValue, ModuleNodeValue, ParameterNode,
        ParameterNodeValue, StatementNodeValue, TypeExpressionNodeValue,
    };

    pub fn x(x: ExpressionNodeValue<()>) -> ExpressionNode<()> {
        ExpressionNode::raw(x, RANGE)
    }

    pub fn xr(x: ExpressionNodeValue<()>, (start, end): InitRange) -> ExpressionNode<()> {
        ExpressionNode::raw(x, Range(start, end))
    }

    pub fn xc<T>(x: ExpressionNodeValue<T>, ctx: T) -> ExpressionNode<T> {
        ExpressionNode(Node::new(x, RANGE, ctx))
    }

    pub fn s(x: StatementNodeValue<()>) -> StatementNode<()> {
        StatementNode::raw(x, RANGE)
    }

    pub fn sr(x: StatementNodeValue<()>, (start, end): InitRange) -> StatementNode<()> {
        StatementNode::raw(x, Range(start, end))
    }

    pub fn sc<T>(x: StatementNodeValue<T>, ctx: T) -> StatementNode<T> {
        StatementNode(Node::new(x, RANGE, ctx))
    }

    pub fn kx(x: KSXNodeValue<()>) -> KSXNode<()> {
        KSXNode::raw(x, RANGE)
    }

    pub fn kxr(x: KSXNodeValue<()>, (start, end): InitRange) -> KSXNode<()> {
        KSXNode::raw(x, Range(start, end))
    }

    pub fn kxc<T>(x: KSXNodeValue<T>, ctx: T) -> KSXNode<T> {
        KSXNode(Node::new(x, RANGE, ctx))
    }

    pub fn p(x: ParameterNodeValue<()>) -> ParameterNode<()> {
        ParameterNode::raw(x, RANGE)
    }

    pub fn pr(x: ParameterNodeValue<()>, (start, end): InitRange) -> ParameterNode<()> {
        ParameterNode::raw(x, Range(start, end))
    }

    pub fn pc<T>(x: ParameterNodeValue<T>, ctx: T) -> ParameterNode<T> {
        ParameterNode(Node::new(x, RANGE, ctx))
    }

    pub fn tx(x: TypeExpressionNodeValue<()>) -> TypeExpressionNode<()> {
        TypeExpressionNode::raw(x, RANGE)
    }

    pub fn txr(x: TypeExpressionNodeValue<()>, (start, end): InitRange) -> TypeExpressionNode<()> {
        TypeExpressionNode::raw(x, Range(start, end))
    }

    pub fn txc<T>(x: TypeExpressionNodeValue<T>, ctx: T) -> TypeExpressionNode<T> {
        TypeExpressionNode(Node::new(x, RANGE, ctx))
    }

    pub fn d(x: DeclarationNodeValue<()>) -> DeclarationNode<()> {
        DeclarationNode::raw(x, RANGE)
    }

    pub fn dr(x: DeclarationNodeValue<()>, (start, end): InitRange) -> DeclarationNode<()> {
        DeclarationNode::raw(x, Range(start, end))
    }

    pub fn dc<T>(x: DeclarationNodeValue<T>, ctx: T) -> DeclarationNode<T> {
        DeclarationNode(Node::new(x, RANGE, ctx))
    }

    pub fn mr(x: ModuleNodeValue<()>) -> ModuleNode<()> {
        ModuleNode::raw(x)
    }
}

/// ast factories
#[allow(dead_code)]
pub mod a {
    use super::*;
    use crate::ast::storage::{Storage, Visibility};

    pub fn type_<E, P, M, T>(name: &str, value: T) -> Declaration<E, P, M, T> {
        Declaration::TypeAlias {
            name: Storage(Visibility::Public, name.to_string()),
            value,
        }
    }

    pub fn enum_<E, P, M, T>(
        name: &str,
        variants: Vec<(String, Vec<T>)>,
    ) -> Declaration<E, P, M, T> {
        Declaration::Enumerated {
            name: Storage(Visibility::Public, name.to_string()),
            variants,
        }
    }

    pub fn const_<E, P, M, T>(
        name: &str,
        value_type: Option<T>,
        value: E,
    ) -> Declaration<E, P, M, T> {
        Declaration::Constant {
            name: Storage(Visibility::Public, name.to_string()),
            value_type,
            value,
        }
    }

    pub fn func_<E, P, M, T>(
        name: &str,
        parameters: Vec<P>,
        body_type: Option<T>,
        body: E,
    ) -> Declaration<E, P, M, T> {
        Declaration::Function {
            name: Storage(Visibility::Public, name.to_string()),
            parameters,
            body_type,
            body,
        }
    }

    pub fn view<E, P, M, T>(name: &str, parameters: Vec<P>, body: E) -> Declaration<E, P, M, T> {
        Declaration::View {
            name: Storage(Visibility::Public, name.to_string()),
            parameters,
            body,
        }
    }

    pub fn module<E, P, M, T>(name: &str, value: M) -> Declaration<E, P, M, T> {
        Declaration::Module {
            name: Storage(Visibility::Public, name.to_string()),
            value,
        }
    }
}
