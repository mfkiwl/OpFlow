// ----------------------------------------------------------------------------
//
// Copyright (c) 2019 - 2021 by the OpFlow developers
//
// This file is part of OpFlow.
//
// OpFlow is free software and is distributed under the MPL v2.0 license.
// The full text of the license can be found in the file LICENSE at the top
// level directory of OpFlow.
//
// ----------------------------------------------------------------------------

#ifndef OPFLOW_LOGICALBC_HPP
#define OPFLOW_LOGICALBC_HPP

#include "Core/BC/BCBase.hpp"
#include "Core/Field/MeshBased/Structured/StructuredFieldExprTrait.hpp"
#include "DataStructures/Range/Ranges.hpp"

namespace OpFlow {
    template <FieldExprType F>
    struct LogicalBCBase : virtual public BCBase<F> {
    public:
        void rebindField(const F& f) { _f = &f; };

    protected:
        const F* _f = nullptr;
    };

    template <StructuredFieldExprType F>
    struct SymmBC : LogicalBCBase<F> {
    protected:
        BCType type = BCType::Symm;

    public:
        SymmBC() = default;
        explicit SymmBC(const F& f, int dim, DimPos pos) : dim(dim), pos(pos) { this->_f = &f; }

        typename internal::StructuredFieldExprTrait<F>::elem_type
        evalAt(const typename internal::StructuredFieldExprTrait<F>::index_type& index) const override {
            auto reflected = reflectIndex(index);
            OP_ASSERT_MSG(
                    DS::inRange(DS::commonRange(this->_f->accessibleRange, this->_f->localRange), reflected),
                    "Reflected index {} out of range {}", reflected.toString(),
                    DS::commonRange(this->_f->accessibleRange, this->_f->localRange).toString());
            auto ret = this->_f->evalAt(reflected);
            return ret;
        }

        [[nodiscard]] BCType getBCType() const override { return BCType::Symm; }
        [[nodiscard]] std::string getTypeName() const override { return "SymmetricBC"; }
        [[nodiscard]] std::string toString(int level) const override {
            std::string ret, prefix;
            for (auto i = 0; i < level; ++i) prefix += "\t";
            ret += prefix + "Type: Symmetric";
            return ret;
        }

        std::unique_ptr<BCBase<F>> getCopy() const override { return std::make_unique<SymmBC>(*this); }

    protected:
        auto reflectIndex(const typename internal::StructuredFieldExprTrait<F>::index_type& index) const {
            auto ret = index;
            auto start = pos == DimPos::start;
            auto corner = this->_f->loc[dim] == LocOnMesh::Corner;

            if (start) {
                ret[dim] = 2 * this->_f->accessibleRange.start[dim] - index[dim];
            } else if (corner) {
                ret[dim] = 2 * (this->_f->accessibleRange.end[dim] - 1) - index[dim];
            } else {// !start && !corner
                ret[dim] = 2 * (this->_f->accessibleRange.end[dim]) - 1 - index[dim];
            }
            return ret;
        }
        void assignImpl(const BCBase<F>& other) override {
            OP_ASSERT_MSG(other.getBCType() == BCType::Symm, "Trying to assign a {} typed BC to SymmBC.",
                          other.getTypeName());
            const auto& _other = reinterpret_cast<const SymmBC<F>&>(other);
            this->_f = _other._f;
            dim = _other.dim;
            pos = _other.pos;
        }

        int dim = -1;
        DimPos pos = DimPos::start;
    };

    template <StructuredFieldExprType F>
    struct ASymmBC : LogicalBCBase<F> {
    protected:
        BCType type = BCType::Symm;

    public:
        ASymmBC() = default;
        explicit ASymmBC(const F& f, int dim, DimPos pos) : dim(dim), pos(pos) { this->_f = &f; }

        typename internal::StructuredFieldExprTrait<F>::elem_type
        evalAt(const typename internal::StructuredFieldExprTrait<F>::index_type& index) const override {
            auto reflected = reflectIndex(index);
            OP_ASSERT_MSG(
                    DS::inRange(DS::commonRange(this->_f->accessibleRange, this->_f->localRange), reflected),
                    "Reflected index {} out of range {}", reflected.toString(),
                    DS::commonRange(this->_f->accessibleRange, this->_f->localRange).toString());
            OP_ASSERT_MSG((reflected != index || this->_f->evalAt(index) == 0),
                          "ASymmetric BC specified on a boundary which is evaluated to {} != 0.",
                          this->_f->evalAt(index));
            return -this->_f->evalAt(reflected);
        }

        [[nodiscard]] BCType getBCType() const override { return BCType::ASymm; }
        [[nodiscard]] std::string getTypeName() const override { return "ASymmetricBC"; }
        [[nodiscard]] std::string toString(int level) const override {
            std::string ret, prefix;
            for (auto i = 0; i < level; ++i) prefix += "\t";
            ret += prefix + "Type: ASymmetric";
            return ret;
        }

        std::unique_ptr<BCBase<F>> getCopy() const override { return std::make_unique<ASymmBC>(*this); }

    protected:
        auto reflectIndex(const typename internal::StructuredFieldExprTrait<F>::index_type& index) const {
            auto ret = index;
            auto start = pos == DimPos::start;
            auto corner = this->_f->loc[dim] == LocOnMesh::Corner;

            if (start) {
                ret[dim] = 2 * this->_f->accessibleRange.start[dim] - index[dim];
            } else if (!start && corner) {
                ret[dim] = 2 * (this->_f->accessibleRange.end[dim] - 1) - index[dim];
            } else {// !start && !corner
                ret[dim] = 2 * (this->_f->accessibleRange.end[dim]) - 1 - index[dim];
            }
            return ret;
        }

        void assignImpl(const BCBase<F>& other) override {
            OP_ASSERT_MSG(other.getBCType() == BCType::ASymm, "Trying to assign a {} typed BC to ASymmBC.",
                          other.getTypeName());
            const auto& _other = reinterpret_cast<const ASymmBC<F>&>(other);
            this->_f = _other._f;
            dim = _other.dim;
            pos = _other.pos;
        }

        int dim = -1;
        DimPos pos = DimPos::start;
    };

}// namespace OpFlow
#endif//OPFLOW_LOGICALBC_HPP
