# Copyright 1999-2011 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

# Install as sys-apps/procmon in portage overlay and run:
# bash> ebuild procmon-x.y.z.ebuild digest

EAPI=4

inherit eutils

DESCRIPTION="Process monitor and system health maintainer application"
HOMEPAGE="http://it.bmc.uu.se/andlov/proj/procmon/"
SRC_URI="http://it.bmc.uu.se/andlov/proj/procmon/stable/${P}.tar.gz"
LICENSE="GPL-3"

SLOT="0"
KEYWORDS="~x86 ~amd64"

IUSE=""
DEPEND="sys-process/procps"
RDEPEND="${DEPEND}"
