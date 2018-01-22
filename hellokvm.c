#include <linux/kvm.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define RAM_SIZE 128*1024*1024

#include "code.h"

int main(int argc, char **argv)
{
    struct kvm_userspace_memory_region mem;
    struct kvm_run *vcpu;
    int kvmfd, vmfd, vcpufd;
    int vcpu_size;
    void *vmram;

    kvmfd = open("/dev/kvm", O_RDWR);
    if (kvmfd < 0) {
        perror("error opening kvm: ");
        return -1;
    }

    vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0);
    if (vmfd < 0) {
        perror("error creating VM: ");
        return -1;
    }

    vmram = mmap(NULL, RAM_SIZE, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (vmram == MAP_FAILED) {
        perror("error mmaping ram: ");
        return -1;
    }

    memcpy(vmram, guest_code, sizeof(guest_code));

    mem.slot = 0;
    mem.flags = 0;
    mem.guest_phys_addr = 0;
    mem.memory_size = RAM_SIZE;
    mem.userspace_addr = (__u64) vmram;

    if (ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &mem) < 0) {
        perror("error setting memory region: ");
        return -1;
    }

    vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
    if (vcpufd < 0) {
        perror("error creating vcpu: ");
        return -1;
    }

    vcpu_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, 0);
    if (vcpu_size < 0) {
        perror("error getting vCPU mmap size: ");
        return -1;
    }

    vcpu = mmap(NULL, vcpu_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpufd, 0);
    if (vcpu == MAP_FAILED) {
        perror("error mapping vcpu: ");
        return -1;
    }

    while (1) {
        if (ioctl(vcpufd, KVM_RUN, 0) < 0) {
            perror("error running vCPU: ");
            return -1;
        }
            
        switch (vcpu->exit_reason) {
        case KVM_EXIT_INTERNAL_ERROR:
            printf("internal_error\n");
            break;
        case KVM_EXIT_IO:
            printf("data: %c\n", *(int *)((char *)vcpu + vcpu->io.data_offset));
            break;
        case KVM_EXIT_HLT:
            printf("halt\n");
            return 0;
        default:
            printf("exit_reason: 0x%x\n", vcpu->exit_reason);
        }
    }
}
