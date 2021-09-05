# Copyright 2021 Takagi Isamu
#
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

class InterfaceSpecification(object):

    def __init__(self, socket, info):
        self._socket = socket
        self._name = info['name']
        self._type = info['type']
        self._data = tuple(info['data'].split('/'))

    @property
    def socket(self):
        return self._socket

    @property
    def name(self):
        return self._name

    @property
    def type(self):
        return self._type

    @property
    def data(self):
        return self._data
