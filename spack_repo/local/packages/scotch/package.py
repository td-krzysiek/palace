# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from spack_repo.builtin.packages.scotch.package import Scotch as BuiltinScotch

from spack.package import *


class Scotch(BuiltinScotch):
    """Scotch, in this repository so that Palace can apply its patches to it."""
